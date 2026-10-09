#include <tests/lib/TestSetup.h>
#include <zypp/ProblemSolution.h>
#include <zypp/ResPool.h>
#include <zypp/ResPoolProxy.h>
#include <zypp/pool/PoolStats.h>
#include <zypp/ui/Selectable.h>

#define BOOST_TEST_MODULE Dup

/////////////////////////////////////////////////////////////////////////////

static TestSetup test(TestSetup::initLater);
struct TestInit {
  TestInit() {
    test = TestSetup(Arch_x86_64);
    test.loadTestcaseRepos(TESTS_SRC_DIR "/data/TCdup");
    dumpRange(USR, test.pool().knownRepositoriesBegin(),
              test.pool().knownRepositoriesEnd())
        << endl;
  }
  ~TestInit() { test.reset(); }
};

template <class TIterator>
std::ostream &vdumpPoolStats(std::ostream &str, TIterator begin_r,
                             TIterator end_r) {
  pool::PoolStats stats;
  for_(it, begin_r, end_r) {
    str << *it << endl;
    stats(*it);
  }
  return str << stats;
}

bool upgrade() {
  bool rres = false;
  {
    rres = getZYpp()->resolver()->doUpgrade();
  }
  if (!rres) {
    ERR << "upgrade " << rres << endl;
    getZYpp()->resolver()->problems();
    return false;
  }
  MIL << "upgrade " << rres << endl;
  vdumpPoolStats(USR << "Transacting:" << endl,
                 make_filter_begin<resfilter::ByTransact>(test.pool()),
                 make_filter_end<resfilter::ByTransact>(test.pool()))
      << endl;
  return true;
}

BOOST_GLOBAL_FIXTURE(TestInit);

BOOST_AUTO_TEST_CASE(orphaned) {
  USR << "pool: " << test.pool() << endl;
  BOOST_REQUIRE(upgrade());

  ResPoolProxy proxy(test.poolProxy());
  BOOST_CHECK_EQUAL(proxy.lookup(ResKind::package, "glibc")->status(),
                    ui::S_KeepInstalled);
  BOOST_CHECK_EQUAL(proxy.lookup(ResKind::package, "release-package")->status(),
                    ui::S_AutoUpdate);
  BOOST_CHECK_EQUAL(
      proxy.lookup(ResKind::package, "dropped_required")->status(),
      ui::S_KeepInstalled);
  BOOST_CHECK_EQUAL(proxy.lookup(ResKind::package, "dropped")->status(),
                    ui::S_AutoDel);
}

BOOST_AUTO_TEST_CASE(keepObsoleteSolution) {
  // dist-upgrade problem whose 'keep obsolete' solution must be
  // classified by locksInstalledOnly/getIfLocksInstalledOnly:
  //   - the update repo offers glibc 2 and bar 2 (requiring glibc = 1)
  //   - so replacing the installed glibc 1 can never satisfy bar 2
  //   - the solver reports the problem and offers 'keep obsolete glibc'
  //     (LOCK action) next to deinstalling bar (KEEP) and ignoring
  //     dependencies (inject).
  TestSetup ktest(Arch_x86_64);
  ktest.loadTestcaseRepos(TESTS_SRC_DIR "/data/TCKeepObsolete");

  ResPool kpool(ktest.pool());
  PoolItem iglibc;
  for (const auto &pi : kpool.byName("glibc")) {
    if (pi.isSystem())
      iglibc = pi;
  }
  BOOST_REQUIRE(iglibc);

  Resolver &resolver(ktest.resolver());
  BOOST_REQUIRE(!resolver.doUpgrade());
  ResolverProblemList rproblems(resolver.problems());
  BOOST_REQUIRE(!rproblems.empty());

  ProblemSolution_Ptr keepObs;
  unsigned keepObsCount = 0;
  for (const auto &prob : rproblems) {
    USR << prob << endl;
    for (const auto &sol : prob->solutions()) {
      if (sol->locksInstalledOnly()) {
        keepObs = sol;
        ++keepObsCount;
      } else {
        BOOST_CHECK(!sol->getIfLocksInstalledOnly());
      }
    }
  }
  BOOST_REQUIRE(keepObs);
  BOOST_CHECK_EQUAL(keepObsCount, 1U);

  std::optional<std::set<PoolItem>> items{keepObs->getIfLocksInstalledOnly()};
  BOOST_REQUIRE(items);
  BOOST_CHECK_EQUAL(items->size(), 1U);
  BOOST_CHECK_EQUAL(items->count(iglibc), 1U);

  // Applying the solution locks glibc in place (session lock, not
  // saved permanently) so the re-solve reports no further problem.
  ProblemSolutionList apply{keepObs};
  resolver.applySolutions(apply);
  BOOST_CHECK(resolver.doUpgrade());
  BOOST_CHECK(iglibc.status().isLocked());
  BOOST_CHECK(iglibc.status().isByApplHigh());
  BOOST_CHECK(!iglibc.status().transacts());

  ktest.reset();
}

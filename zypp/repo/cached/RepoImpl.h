/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_RepoImpl_H
#define ZYPP_RepoImpl_H

#include <iosfwd>
#include <map>
#include <utility>
#include "zypp/Arch.h"
#include "zypp/Rel.h"
#include "zypp/Pathname.h"
#include "zypp/ProgressData.h"
#include "zypp/data/RecordId.h"
#include "zypp/repo/RepositoryImpl.h"
#include "zypp/ResStore.h"
#include "zypp/RepoInfo.h"
#include "zypp/cache/CacheTypes.h"
#include "zypp/cache/ResolvableQuery.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace cached
    { /////////////////////////////////////////////////////////////////

      struct RepoOptions
      {
        RepoOptions( const RepoInfo &repoinfo_,
                     const Pathname &solvdir_ )

          : repoinfo(repoinfo_)
          , solvdir(solvdir_)
        {}


        ProgressData::ReceiverFnc readingResolvablesProgress;
        ProgressData::ReceiverFnc readingPatchDeltasProgress;
        RepoInfo repoinfo;
        Pathname solvdir;
      };

      /**
       * \short Cached repository implementation
       *
       * Reads attributes on demand from cache
       */
      class RepoImpl : public repo::RepositoryImpl
      {
      public:
        typedef intrusive_ptr<RepoImpl>       Ptr;
        typedef intrusive_ptr<const RepoImpl> constPtr;

      public:
        /** Default ctor */
        RepoImpl( const RepoOptions &opts );
        /** Dtor */
        ~RepoImpl();

      public:
        virtual void createResolvables();
        virtual void createPatchAndDeltas();

        cache::ResolvableQuery resolvableQuery();
      private:
        cache::ResolvableQuery _rquery;
        RepoOptions _options;
      };
      ///////////////////////////////////////////////////////////////////

      /////////////////////////////////////////////////////////////////
    } // namespace cached
    ///////////////////////////////////////////////////////////////////

    using cached::RepoImpl;

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_PLAINDIR_PLAINDIRIMPL_H


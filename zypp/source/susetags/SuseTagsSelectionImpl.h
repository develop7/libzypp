/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/detail/SelectionImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SUSETAGS_SELECTIONIMPL_H
#define ZYPP_DETAIL_SUSETAGS_SELECTIONIMPL_H

#include "zypp/detail/SelectionImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
  namespace susetags
  {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SelectionImpl
    //
    /**
    */
    class SuseTagsSelectionImpl : public zypp::detail::SelectionImplIf
    {
    public:
      SuseTagsSelectionImpl();
      virtual ~SuseTagsSelectionImpl();

    private:
      /*
      virtual std::list<std::string> suggests() const;
      virtual std::list<std::string> recommends() const;
      virtual std::list<std::string> insnotify( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> delnotify( const LangCode & lang = LangCode("") ) const;
      virtual ByteCount size() const;
      virtual bool providesSources() const;
      virtual std::string instSrcLabel() const;
      virtual Vendor instSrcVendor() const;
      virtual unsigned instSrcRank() const;
      virtual std::list<PMSelectionPtr> suggests_ptrs() const;
      virtual std::list<PMSelectionPtr> recommends_ptrs() const;
      virtual std::list<std::string> inspacks( const LangCode & lang = LangCode("") ) const;
      virtual std::list<std::string> delpacks( const LangCode & lang = LangCode("") ) const;
      virtual PM::LocaleSet supportedLocales() const;
      virtual std::set<PMSelectablePtr> pureInspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> inspacks_ptrs( const LangCode & lang ) const;
      virtual std::set<PMSelectablePtr> delpacks_ptrs( const LangCode & lang ) const;
      virtual bool isBase() const;
      virtual PMError provideSelToInstall( Pathname & path_r ) const;
      */
      std::set<std::string> _suggests;
      std::set<std::string> _recommends;
      std::set<std::string> _supported_locales;
      std::map< std::string, std::set<std::string> > _insnotify;
      std::map< std::string, std::set<std::string> > _delnotify;
      std::map< std::string, std::set<std::string> > _inspacks;
      std::map< std::string, std::set<std::string> > _delpacks;
      
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace susetags
  ///////////////////////////////////////////////////////////////////
  } // namespace source
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SELECTIONIMPL_H

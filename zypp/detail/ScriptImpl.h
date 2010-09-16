/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/detail/ScriptImpl.h
 *
*/
#ifndef ZYPP_DETAIL_SCRIPTIMPL_H
#define ZYPP_DETAIL_SCRIPTIMPL_H

#include "zypp/detail/ScriptImplIf.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ScriptImpl
    //
    /** Class representing an update script */
    class ScriptImpl : public ScriptImplIf
    {
    public:
      /** Default ctor */
      ScriptImpl();
      /** Dtor */
      ~ScriptImpl();

    public:
      /** Get the script to perform the change */
      Pathname do_script() const;
      /** Get the script to undo the change */
      Pathname undo_script() const;
      /** Check whether script to undo the change is available */
      virtual bool undo_available() const;
    protected:
      /** The script to perform the change */
      std::string _do_script;
      /** The script to undo the change */
      std::string _undo_script;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DETAIL_SCRIPTIMPL_H
/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/Hal.cc
 *
 *  \brief Hardware abstaction layer library wrapper implementation.
 */
#ifndef FAKE_HAL // disables zypp's HAL dependency

#include <zypp/target/hal/HalContext.h>
#include <zypp/thread/Mutex.h>
#include <zypp/thread/MutexLock.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/Logger.h>

#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>
#include <hal/libhal.h>
#include <hal/libhal-storage.h>

#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////
namespace zypp
{ ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
  namespace target
  { //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    namespace hal
    { ////////////////////////////////////////////////////////////////

      using zypp::thread::Mutex;
      using zypp::thread::MutexLock;

      ////////////////////////////////////////////////////////////////
      namespace // anonymous
      { //////////////////////////////////////////////////////////////


        //////////////////////////////////////////////////////////////
        // STATIC
        /**
         ** hmm... currently a global one..
        */
        static Mutex g_Mutex;


        //////////////////////////////////////////////////////////////
        /**
         * Internal hal (dbus ) error helper class.
         */
        class HalError
        {
        public:
          DBusError error;

          HalError()  { dbus_error_init(&error); }
          ~HalError() { dbus_error_free(&error); }

          inline std::string toString() const
          {
            if( error.name != NULL && error.message != NULL) {
              return std::string(error.name) +
                     std::string(": ")       +
                     std::string(error.message);
            } else {
              return std::string();
            }
          }
        };


        // -----------------------------------------------------------
        inline void
        VERIFY_CONTEXT(const zypp::RW_pointer<HalContext_Impl> &h)
        {
          if( !h)
          {
            ZYPP_THROW(HalException("HalContext not connected"));
          }
        }

        // -----------------------------------------------------------
        inline void
        VERIFY_DRIVE(const zypp::RW_pointer<HalDrive_Impl> &d)
        {
          if( !d)
          {
            ZYPP_THROW(HalException("HalDrive not initialized"));
          }
        }

        // -----------------------------------------------------------
        inline void
        VERIFY_VOLUME(const zypp::RW_pointer<HalVolume_Impl> &v)
        {
          if( !v)
          {
            ZYPP_THROW(HalException("HalVolume not initialized"));
          }
        }

        //////////////////////////////////////////////////////////////
      } // anonymous
      ////////////////////////////////////////////////////////////////


      ////////////////////////////////////////////////////////////////
      class HalContext_Impl
      {
      public:
        HalContext_Impl(bool monitorable = false);
        ~HalContext_Impl();

        DBusConnection *conn;
        LibHalContext  *hctx;
      };


      ////////////////////////////////////////////////////////////////
      class HalDrive_Impl
      {
      public:
        zypp::RW_pointer<HalContext_Impl>  hal;
        LibHalDrive                       *drv;

        HalDrive_Impl()
          : hal(), drv(NULL)
        {
        }

        HalDrive_Impl(const zypp::RW_pointer<HalContext_Impl> &r,
                      LibHalDrive *d)
          : hal(r), drv(d)
        {
        }

        ~HalDrive_Impl()
        {
          if( drv)
            libhal_drive_free(drv);
        }
      };


      ////////////////////////////////////////////////////////////////
      class HalVolume_Impl
      {
      public:
        LibHalVolume *vol;

        HalVolume_Impl(LibHalVolume *v=NULL)
          : vol(v)
        {
        }

        ~HalVolume_Impl()
        {
          if( vol)
            libhal_volume_free(vol);
        }
      };


      ////////////////////////////////////////////////////////////////
      HalContext_Impl::HalContext_Impl(bool monitorable)
        : conn(NULL)
        , hctx(NULL)
      {
        HalError err;

        conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err.error);
        if( !conn) {
          ZYPP_THROW(HalException(err.toString()));
        }

        if( monitorable)
          dbus_connection_setup_with_g_main(conn, NULL);

        hctx = libhal_ctx_new();
        if( !hctx)
        {
          dbus_connection_disconnect(conn);
          dbus_connection_unref(conn);
          conn = NULL;

          ZYPP_THROW(HalException(
            "libhal_ctx_new: Can't create libhal context"
          ));
        }

        if( !libhal_ctx_set_dbus_connection(hctx, conn))
        {
          libhal_ctx_free(hctx);
          hctx = NULL;

          dbus_connection_disconnect(conn);
          dbus_connection_unref(conn);
          conn = NULL;

          ZYPP_THROW(HalException(
            "libhal_set_dbus_connection: Can't set dbus connection"
          ));
        }

        if( !libhal_ctx_init(hctx, &err.error))
        {
          libhal_ctx_free(hctx);
          hctx = NULL;

          dbus_connection_disconnect(conn);
          dbus_connection_unref(conn);
          conn = NULL;

          ZYPP_THROW(HalException(err.toString()));
        }
      }

      // -------------------------------------------------------------
      HalContext_Impl::~HalContext_Impl()
      {
        if( hctx)
        {
          HalError err;
          libhal_ctx_shutdown(hctx, &err.error);
          libhal_ctx_free( hctx);
        }
        if( conn)
        {
          dbus_connection_disconnect(conn);
          dbus_connection_unref(conn);
        }
      }


      ////////////////////////////////////////////////////////////////
      HalContext::HalContext(bool autoconnect)
        : h_impl( NULL)
      {
        MutexLock lock(g_Mutex);

        if( autoconnect)
          h_impl.reset( new HalContext_Impl());
      }

      // -------------------------------------------------------------
      HalContext::HalContext(const HalContext &context)
        : h_impl( NULL)
      {
        MutexLock lock(g_Mutex);

        zypp::RW_pointer<HalContext_Impl>(context.h_impl).swap(h_impl);
      }

      // -------------------------------------------------------------
      HalContext::HalContext(bool autoconnect, bool monitorable)
        : h_impl( NULL)
      {
        MutexLock lock(g_Mutex);

        if( autoconnect)
          h_impl.reset( new HalContext_Impl(monitorable));
      }

      // -------------------------------------------------------------
      HalContext::~HalContext()
      {
        MutexLock  lock(g_Mutex);

        h_impl.reset();
      }

      // --------------------------------------------------------------
      HalContext &
      HalContext::operator=(const HalContext &context)
      {
        MutexLock  lock(g_Mutex);

        if( this == &context)
          return *this;

        zypp::RW_pointer<HalContext_Impl>(context.h_impl).swap(h_impl);
      }

      // --------------------------------------------------------------
      HalContext::operator HalContext::bool_type() const
      {
        MutexLock  lock(g_Mutex);

        return h_impl;
      }

      // --------------------------------------------------------------
      void
      HalContext::connect()
      {
        MutexLock lock(g_Mutex);

        if( !h_impl)
          h_impl.reset( new HalContext_Impl());
      }

      // --------------------------------------------------------------
      std::vector<std::string>
      HalContext::getAllDevices() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError   err;
        char     **names;
        int        count = 0;

        names = libhal_get_all_devices( h_impl->hctx, &count, &err.error);
        if( !names)
        {
          ZYPP_THROW(HalException(err.toString()));
        }

        std::vector<std::string> ret(names, names + count);
        libhal_free_string_array(names);
        return ret;
      }

      // --------------------------------------------------------------
      std::vector<std::string>
      HalContext::findDevicesByCapability(const std::string &capability) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        HalError   err;
        char     **names;
        int        count = 0;

        names = libhal_find_device_by_capability(h_impl->hctx,
                                                 capability.c_str(),
                                                 &count, &err.error);
        if( !names)
        {
          ZYPP_THROW(HalException(err.toString()));
        }

        std::vector<std::string> ret(names, names + count);
        libhal_free_string_array(names);
        return ret;
      }

      // --------------------------------------------------------------
      HalDrive
      HalContext::getDriveFromUDI(const std::string &udi) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        LibHalDrive *drv = libhal_drive_from_udi(h_impl->hctx, udi.c_str());
        if( drv != NULL)
          return HalDrive(new HalDrive_Impl( h_impl, drv));
        else
          return HalDrive();
      }
      
      // --------------------------------------------------------------
      HalVolume
      HalContext::getVolumeFromUDI(const std::string &udi) const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_CONTEXT(h_impl);

        LibHalVolume *vol = libhal_volume_from_udi(h_impl->hctx, udi.c_str());
        if( vol)
          return HalVolume( new HalVolume_Impl(vol));
        else
          return HalVolume();
      }


      ////////////////////////////////////////////////////////////////
      HalDrive::HalDrive()
        : d_impl( NULL)
      {
      }

      // --------------------------------------------------------------
      HalDrive::HalDrive(HalDrive_Impl *impl)
        : d_impl( NULL)
      {
        MutexLock  lock(g_Mutex);

        d_impl.reset(impl);
      }

      // --------------------------------------------------------------
      HalDrive::HalDrive(const HalDrive &drive)
        : d_impl( NULL)
      {
        MutexLock  lock(g_Mutex);

        zypp::RW_pointer<HalDrive_Impl>(drive.d_impl).swap(d_impl);
      }

      // --------------------------------------------------------------
      HalDrive::~HalDrive()
      {
        MutexLock  lock(g_Mutex);

        d_impl.reset();
      }

      // --------------------------------------------------------------
      HalDrive &
      HalDrive::operator=(const HalDrive &drive)
      {
        MutexLock  lock(g_Mutex);

        if( this == &drive)
          return *this;

        zypp::RW_pointer<HalDrive_Impl>(drive.d_impl).swap(d_impl);
      }

      // --------------------------------------------------------------
      HalDrive::operator HalDrive::bool_type() const
      {
        MutexLock  lock(g_Mutex);

        return d_impl;
      }

      // --------------------------------------------------------------
      std::vector<std::string>
      HalDrive::findAllVolumes() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        char     **names;
        int        count = 0;

        names = libhal_drive_find_all_volumes(d_impl->hal->hctx,
                                              d_impl->drv,
                                              &count);

        std::vector<std::string> ret;
        ret.assign(names, names + count);
        libhal_free_string_array(names);
        return ret;
      }

      // --------------------------------------------------------------
      bool
      HalDrive::usesRemovableMedia() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        return libhal_drive_uses_removable_media(d_impl->drv);
      }

      // --------------------------------------------------------------
      std::string
      HalDrive::getDeviceFile() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        return std::string(libhal_drive_get_device_file(d_impl->drv));
      }

      // --------------------------------------------------------------
      unsigned int
      HalDrive::getDeviceMajor() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        return libhal_drive_get_device_major(d_impl->drv);
      }

      // --------------------------------------------------------------
      unsigned int
      HalDrive::getDeviceMinor() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_DRIVE(d_impl);

        return libhal_drive_get_device_minor(d_impl->drv);
      }


      ////////////////////////////////////////////////////////////////
      HalVolume::HalVolume()
        : v_impl( NULL)
      {}

      HalVolume::HalVolume(HalVolume_Impl *impl)
        : v_impl( NULL)
      {
        MutexLock  lock(g_Mutex);

        v_impl.reset(impl);
      }

      // --------------------------------------------------------------
      HalVolume::HalVolume(const HalVolume &volume)
        : v_impl( NULL)
      {
        MutexLock  lock(g_Mutex);

        zypp::RW_pointer<HalVolume_Impl>(volume.v_impl).swap(v_impl);
      }

      // --------------------------------------------------------------
      HalVolume::~HalVolume()
      {
        MutexLock  lock(g_Mutex);

        v_impl.reset();
      }

      // --------------------------------------------------------------
      HalVolume &
      HalVolume::operator=(const HalVolume &volume)
      {
        MutexLock  lock(g_Mutex);

        if( this == &volume)
          return *this;

        zypp::RW_pointer<HalVolume_Impl>(volume.v_impl).swap(v_impl);
      }

      // --------------------------------------------------------------
      HalVolume::operator HalVolume::bool_type() const
      {
        MutexLock  lock(g_Mutex);

        return v_impl;
      }

      // --------------------------------------------------------------
      std::string
      HalVolume::getDeviceFile() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return std::string(libhal_volume_get_device_file(v_impl->vol));
      }

      // --------------------------------------------------------------
      unsigned int
      HalVolume::getDeviceMajor() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return libhal_volume_get_device_major(v_impl->vol);
      }

      // --------------------------------------------------------------
      unsigned int
      HalVolume::getDeviceMinor() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return libhal_volume_get_device_minor(v_impl->vol);
      }

      // --------------------------------------------------------------
      std::string
      HalVolume::getFSType() const
      {
        MutexLock  lock(g_Mutex);
        VERIFY_VOLUME(v_impl);

        return std::string( libhal_volume_get_fstype(v_impl->vol));
      }

      // --------------------------------------------------------------
      std::string
      HalVolume::getMountPoint() const
      {
        VERIFY_VOLUME(v_impl);

        return std::string( libhal_volume_get_mount_point(v_impl->vol));
      }


      ////////////////////////////////////////////////////////////////
    } // namespace hal
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
  } // namespace target
  ////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////////
#endif // FAKE_HAL

/*
** vim: set ts=2 sts=2 sw=2 ai et:
*/

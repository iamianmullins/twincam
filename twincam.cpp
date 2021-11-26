#include <kms++/kms++.h>
#include <kms++util/kms++util.h>

#include <fmt/format.h>

#include <fcntl.h>
#include <glob.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>

#include <libcamera/libcamera.h>

#define eprint(...) fprintf(stderr, format(__VA_ARGS__).c_str())

using namespace fmt;
using namespace std;
using namespace libcamera::properties;

using libcamera::Camera;
using libcamera::CameraConfiguration;
using libcamera::CameraManager;
using libcamera::ControlId;
using libcamera::ControlInfo;
using libcamera::ControlInfoMap;
using libcamera::ControlList;
using libcamera::FrameBuffer;
using libcamera::FrameBufferAllocator;
using libcamera::Request;
using libcamera::Size;
using libcamera::Stream;
using libcamera::StreamConfiguration;
using libcamera::StreamFormats;
using libcamera::StreamRole;
using libcamera::StreamRoles;

using kms::AtomicReq;
using kms::Card;
using kms::Connector;
using kms::Crtc;
using kms::DumbFramebuffer;
using kms::Framebuffer;
using kms::Plane;
using kms::PlaneType;
using kms::Videomode;

static const char* usage_str =
    "Usage: twincam [OPTIONS]\n\n"
    "Options:\n"
    "  -l, --list-displays         List displays\n"
    "  -c, --list-cameras          List cameras\n"
    "  -h, --help                  Print this help\n";

static string PixelFormatToCC(const kms::PixelFormat& f) {
  char buf[5] = {(char)(((uint32_t)f >> 0) & 0xff),
                 (char)(((uint32_t)f >> 8) & 0xff),
                 (char)(((uint32_t)f >> 16) & 0xff),
                 (char)(((uint32_t)f >> 24) & 0xff), 0};
  for (int i = 0; i < 5; ++i) {
    if (isspace(buf[i])) {
      buf[i] = 0;
      break;
    }
  }

  return string(buf);
}

template <typename T>
struct TD;
int main(int argc, char** argv) {
  bool list_displays = false;
  bool list_cameras = false;

  OptionSet optionset = {
      Option("l|list-displays", [&]() { list_displays = true; }),
      Option("c|list-cameras", [&]() { list_cameras = true; }),
      Option("h|help",
             [&]() {
               puts(usage_str);
               return -1;
             }),
  };

  optionset.parse(argc, argv);

  if (optionset.params().size() > 0) {
    puts(usage_str);
    return -1;
  }

  Card card;
  if (list_displays) {
    print(
        "Card:\n"
        "is_master: {:d}\n"
        "has_atomic: {:d}\n"
        "has_universal_planes: {:d}\n"
        "has_dumb_buffers: {:d}\n"
        "has_kms: {:d}\n"
        "version_name: {:s}\n\n"
        "\tConnectors:\n",
        card.is_master(), card.has_atomic(), card.has_universal_planes(),
        card.has_dumb_buffers(), card.has_kms(), card.version_name().c_str());
  }

  Plane* plane;
  Crtc* ctrc = card.get_connectors().front()->get_current_crtc();
  for (Connector* c : card.get_connectors()) {
    if (list_displays) {
      print(
          "\tfullname: {:s}\n"
          "\tconnected: {:d}\n"
          "\tconnector_type: {:d}\n"
          "\tconnector_type_id: {:d}\n"
          "\tmmWidth: {:d}\n"
          "\tmmHeight: {:d}\n"
          "\tsubpixel: {:d}\n"
          "\tsubpixel_str: {:s}\n\n",
          c->fullname().c_str(), c->connected(), c->connector_type(),
          c->connector_type_id(), c->mmWidth(), c->mmHeight(), c->subpixel(),
          c->subpixel_str().c_str());
    }

    Crtc* this_ctrc = c->get_current_crtc();
    if (this_ctrc) {
      if (list_displays) {
        print(
            "\t\tget_current_crtc:\n"
            "\t\tbuffer_id: {:d}\n"
            "\t\tx: {:d}\n"
            "\t\ty: {:d}\n"
            "\t\twidth {:d}\n"
            "\t\theight: {:d}\n"
            "\t\tmode_valid: {:d}\n"
            "\t\tlegacy_gamma_size: {:d}\n",
            this_ctrc->buffer_id(), this_ctrc->x(), this_ctrc->y(),
            this_ctrc->width(), this_ctrc->height(), this_ctrc->mode_valid(),
            this_ctrc->legacy_gamma_size());
      }

      for (Plane* p : ctrc->get_possible_planes()) {
        if (p->crtc_id()) {
          if (list_displays) {
            print("\t\tcrtc_id: {:d}\n\n", p->crtc_id());
          }

          plane = p;
          break;
        }
      }
    }

    if (list_displays) {
      if (!c->get_modes().empty()) {
        print("\t\tVideomodes:\n");
      }

      //       : "", c->get_current_crtc()->buffer_id(), c->get_current_crtc() ?
      //       '\n' : '',
      //    join((vector<void*>)c->get_possible_crtcs(), ", "),
      //  c->get_modes().empty() ? "" : "\t\tVideomodes:\n");
      for (Videomode& v : c->get_modes()) {
        print(
            "\t\tto_string_long: {:s}\n"
            "\t\thsync_start: {:d}\n"
            "\t\thsync_end: {:d}\n"
            "\t\thtotal: {:d}\n"
            "\t\thskew: {:d}\n"
            "\t\tvsync_start: {:d}\n"
            "\t\tvsync_end: {:d}\n"
            "\t\tvtotal: {:d}\n"
            "\t\tvscan: {:d}\n"
            "\t\tvalid: {:d}\n\n",
            v.to_string_long().c_str(), v.hsync_start, v.hsync_end, v.htotal,
            v.hskew, v.vsync_start, v.vsync_end, v.vtotal, v.vscan, v.valid());
      }

      print("\tPlanes:\n");
      for (Plane* p : card.get_planes()) {
        print(
            "\tcrtc_id: {:d}\n"
            "\tfb_id: {:d}\n"
            "\tcrtc_x: {:d}\n"
            "\tcrtc_y: {:d}\n"
            "\tx: {:d}\n"
            "\ty: {:d}\n"
            "\tgamma_size: {:d}\n"
            "\tplane_type: {:d}\n"
            "\tPixelFormatToCC:",
            p->crtc_id(), p->fb_id(), p->crtc_x(), p->crtc_y(), p->x(), p->y(),
            p->gamma_size(), p->plane_type());
        for (size_t i = 0; i < p->get_formats().size(); ++i) {
          print("{:s}{:s}", i ? ", " : " ",
                PixelFormatToCC(p->get_formats()[i]).c_str());
        }

        print("\n\n");
      }
    }
  }

  setenv("LIBCAMERA_LOG_LEVELS", "2", 1);
  CameraManager cm;
  int ret = cm.start();
  if (ret) {
    eprint("{:d} = start()\n", ret);
  }

  if (cm.cameras().empty()) {
    print("No cameras present\n");
  } else if (list_cameras) {
    print("Cameras:\n");
  }

  const vector<shared_ptr<Camera>>& cameras = cm.cameras();
  const shared_ptr<Camera>& camera = cameras[0];
  for (size_t cam_num = 0; cam_num < cameras.size(); ++cam_num) {
    const shared_ptr<Camera>& cam = cameras[cam_num];

    if (list_cameras) {
      const ControlList& props = cam->properties();
      print(
          "cam_num: {:d}\n"
          "id: {:s}\n",
          cam_num, cam.get()->id().c_str());

      if (props.contains(Location)) {
        print("Location: ");
        switch (props.get(Location)) {
          case CameraLocationFront:
            print("front");
            break;
          case CameraLocationBack:
            print("back");
            break;
          case CameraLocationExternal:
            print("external");
            break;
        }

        print("\n");
      }

      if (props.contains(Model)) {
        print("Model: {:s}\n", props.get(Model).c_str());
      }

      if (props.contains(PixelArraySize)) {
        print("PixelArraySize: {:s}\n",
              props.get(PixelArraySize).toString().c_str());
      }

      if (props.contains(PixelArrayActiveAreas)) {
        print("PixelArrayActiveAreas: {:s}\n",
              props.get(PixelArrayActiveAreas)[0].toString().c_str());
      }

      for (const pair<const ControlId* const, ControlInfo>& ctrl :
           cam->controls()) {
        const ControlId* id = ctrl.first;
        const ControlInfo& info = ctrl.second;

        print("{:s}: {:s}\n", id->name().c_str(), info.toString().c_str());
      }

      unique_ptr<CameraConfiguration> config =
          cam->generateConfiguration({StreamRole::Viewfinder});
      if (!config) {
        eprint("0 = generateConfiguration()\n");
      }

      switch (config->validate()) {
        case CameraConfiguration::Valid:
          break;

        case CameraConfiguration::Adjusted:
          print("Camera configuration adjusted\n");
          break;

        case CameraConfiguration::Invalid:
          eprint("Camera configuration invalid\n");
          break;
      }

      for (const StreamConfiguration& cfg : *config.get()) {
        print("StreamConfiguration: {:s}\n\n", cfg.toString().c_str());

        const StreamFormats& formats = cfg.formats();
        print("\tPixelFormats:\n");
        for (libcamera::PixelFormat pf : formats.pixelformats()) {
          print("\ttoString: {:s} {:s}\n\tSizes: ", pf.toString().c_str(),
                formats.range(pf).toString().c_str());

          for (size_t i = 0; i < formats.sizes(pf).size(); ++i) {
            print("{:s}{:s}", i ? ", " : "",
                  formats.sizes(pf)[i].toString().c_str());
          }

          print("\n\n");
        }
      }
    }
  }

  if (!ctrc) {
    eprint("no ctrc set\n");
    return 0;
  }

  AtomicReq req(card);
  req.add(plane, "CRTC_ID", plane->crtc_id());
  req.add(plane, "FB_ID", plane->fb_id());

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t iw = ctrc->width();
  uint32_t ih = ctrc->height();
  uint32_t cam_width = camera->properties().get(PixelArraySize).width;
  uint32_t cam_height = camera->properties().get(PixelArraySize).height;
  uint32_t out_x = x + iw / 2 - cam_width / 2;
  uint32_t out_y = y + ih / 2 - cam_height / 2;

  req.add(plane, "CRTC_X", out_x);
  req.add(plane, "CRTC_Y", out_y);
  req.add(plane, "CRTC_W", cam_width);
  req.add(plane, "CRTC_H", cam_height);

  req.add(plane, "SRC_X", 0);
  req.add(plane, "SRC_Y", 0);
  req.add(plane, "SRC_W", cam_width << 16);
  req.add(plane, "SRC_H", cam_height << 16);

  ret = req.commit_sync();
  if (ret) {
    eprint("{:d} = req.commit_sync();\n", ret);
  }

  ret = camera->acquire();  // to be put in C++ class
  if (ret) {
    eprint("{:d} = camera->acquire();\n", ret);
  }

  StreamRoles roles = {StreamRole::Viewfinder};
  std::unique_ptr<libcamera::CameraConfiguration> cfg =
      camera->generateConfiguration(roles);
  if (!cfg) {
    eprint("{:d} = camera->generateConfiguration(roles);\n", ptr(cfg));
  }

  ret = camera->configure(cfg.get());
  if (ret) {
    eprint("{:d} = camera->configure(cfg.get());\n", ret);
  }

  FrameBufferAllocator fba(camera);
  std::vector<std::unique_ptr<libcamera::Request>> requests;
  Stream* stream = 0;
  for (StreamConfiguration& config : *cfg) {
    stream = config.stream();
    ret = fba.allocate(stream);
    if (ret < 0) {
      eprint("{:d} = fba.allocate(stream);\n", ret);
    }

    for (const unique_ptr<FrameBuffer>& buffer : fba.buffers(stream)) {
      std::unique_ptr<Request> request = camera->createRequest();
      if (!request) {
        eprint("{:d} = camera->createRequest();\n", ptr(request));
      }

      ret = request->addBuffer(stream, buffer.get());
      if (ret < 0) {
        eprint("{:d} = request->addBuffer(stream, buffer.get());\n", ret);
      }

      requests.push_back(std::move(request));
      // freeBuffers_[stream].enqueue(buffer.get());
    }
  }

  ret = camera->start();
  if (ret) {
    eprint("{:d} = camera->start();\n", ret);
  }

  // Queue requests
  for (std::unique_ptr<Request>& request : requests) {
    ret = camera->queueRequest(request.get());
    if (ret < 0) {
      eprint("{:d} = camera->queueRequest(request.get())\n", ret);
    }
  }

#if 0
  for (const unique_ptr<FrameBuffer>& buffer : fba.buffers(stream)) {
    AtomicReq req1(card);
    req1.add(plane, "FB_ID", buffer->);
  }
#endif

  ret = camera->release();  // to be put in C++ class
  if (ret) {
    eprint("{:d} = camera->release();\n", ret);
  }

  camera->stop();
  // causes memory leak cm.stop();
}

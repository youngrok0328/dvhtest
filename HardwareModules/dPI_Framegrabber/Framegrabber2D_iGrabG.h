#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Framegrabber_Base.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class Framegrabber2D_iGrabG : public FrameGrabber_Base
{
public:
    Framegrabber2D_iGrabG();
    ~Framegrabber2D_iGrabG() override;

public: // 그랩 함수
    virtual bool grab(const int& grabber_idx, const int& camera_idx, BYTE** image, const int& grab_num,
        Ipvm::AsyncProgress* progress) override;
    virtual bool wait_grab_end(const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* progress) override;

public: //F/W & Lib. Version
    virtual void get_firmware_version(
        const int& grabber_id, unsigned int& cur_version, unsigned int& need_version) override;
    virtual void get_library_version(unsigned int& cur_version, unsigned int& need_version) override;

public: // iGrab Board Temperature
    virtual void set_grabber_temperature(const int& grabber_id = 0) override;

protected:
    virtual void init_grabber_info() override;
    virtual void init_camera_info(const CString& camera_param_file_path) override;
    virtual bool LiveCallback(const int& grabber_id = 0, const int& camera_id = 0) override;
    virtual bool SetTriggerMode(int nMode) override;

private:
    BYTE** grab_buffers;
    int grab_counts;
};

#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxmt.h>

//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//

typedef int grabber_id;
typedef int camera_channel;
typedef int icf_id;

//icf는 camera하나당 하나로 고정한다
struct grabber_info
{
    grabber_id connect_grabber_id; //연결된 grabber id
    std::map<camera_channel, icf_id> camera_icf_id; //순서대로 들어간다
    unsigned int firmware_version;
    float grabber_temperature;

    void init()
    {
        connect_grabber_id = -1;
        camera_icf_id.clear();
        firmware_version = 0;
        grabber_temperature = -1.f;
    }

    void set_firmware_version(const int& grabber_firmware_version)
    {
        firmware_version = grabber_firmware_version;
    }

    unsigned int get_firmware_version() const
    {
        return firmware_version;
    }

    void set_grabber_id(const int& grabber_id)
    {
        connect_grabber_id = grabber_id;
    }

    int get_graaber_id() const
    {
        return connect_grabber_id;
    }

    void set_camera_icf(const camera_channel& id_camera, const icf_id& id_icf)
    {
        camera_icf_id[id_camera] = id_icf;
    }

    int get_connect_camera_num() const
    {
        return (int)camera_icf_id.size();
    }

    int get_connect_channel_id(const int& camera_idx) const
    {
        if (camera_idx >= get_connect_camera_num())
            return -1;

        auto cur = camera_icf_id.begin();
        std::advance(cur, camera_idx);

        return cur->first;
    }

    int get_connect_icf_id(const int& camera_idx) const
    {
        if (camera_idx >= get_connect_camera_num())
            return -1;

        auto cur = camera_icf_id.begin();
        std::advance(cur, camera_idx);

        return cur->second;
    }

    void set_grabber_temperature(const float& temperature)
    {
        grabber_temperature = temperature;
    }

    float get_grabber_temperature() const
    {
        return grabber_temperature;
    }
};
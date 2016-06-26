#pragma once

struct PropertiesMapper
{
    PropertiesMapper() : use_msdp(false) {}
    tstring begin_name;
    tstring end_name;
    tstring begin_descr;
    tstring end_descr;
    tstring begin_exits;
    tstring end_exits;
    tstring begin_prompt;
    tstring end_prompt;
    tstring begin_vnum;
    tstring end_vnum;

    tstring north_cmd;
    tstring south_cmd;
    tstring west_cmd;
    tstring east_cmd;
    tstring up_cmd;
    tstring down_cmd;

    tstring north_exit;
    tstring south_exit;
    tstring west_exit;
    tstring east_exit;
    tstring up_exit;
    tstring down_exit;

    tstring dark_room;

    bool use_msdp;
    
    void initAllDefault()
    {
        begin_name.clear();
        end_name.clear();
        begin_descr.clear();
        end_descr.clear();
        begin_exits.clear();
        end_exits.clear();
        begin_prompt.clear();
        end_prompt.clear();
        
        north_cmd.clear();
        south_cmd.clear();
        west_cmd.clear();
        east_cmd.clear();
        up_cmd.clear();
        down_cmd.clear();

        north_exit.clear();
        south_exit.clear();
        west_exit.clear();
        east_exit.clear();
        up_exit.clear();
        down_exit.clear();

        dark_room.clear();
        use_msdp = false;
    }
};

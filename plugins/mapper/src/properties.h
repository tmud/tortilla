#pragma once

struct PropertiesMapper
{
    PropertiesMapper() {}
    tstring begin_name;
    tstring end_name;
    tstring begin_key;
    tstring end_key;
    tstring begin_descr;
    tstring end_descr;
    tstring begin_exits;
    tstring end_exits;
    tstring begin_prompt;
    tstring end_prompt;

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
    
    void initAllDefault()
    {
        begin_name.clear();
        end_name.clear();
        begin_key.clear();
        end_key.clear();
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
    }
};

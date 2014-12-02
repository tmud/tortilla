#include "stdafx.h"
#include "logicProcessor.h"

void LogicProcessor::processStackTick()
{
    if (m_prompt_mode != OFF)
    {
        MudViewString *last = m_pHost->getLastString(0);
        if (last && !last->prompt && !last->gamecmd && !last->system)
            return;
    }
    printStack(FROM_TIMER);
}

void LogicProcessor::processIncoming(const WCHAR* text, int text_len, int flags, int window)
{
    if (window == 0 && flags & (GAME_LOG|GAME_CMD) && !(flags & FROM_STACK))
    {
       MudViewString *last = m_pHost->getLastString(0);
       if (last && !last->prompt && !last->gamecmd && !last->system)
       {
           // � ����, ���� ������ ����� �������� ������� � ���� (��� prompt/gamecmd, �������� ��� ������ ������).
           stack_el e;
           e.text.assign(text, text_len);
           e.flags = flags;
           m_incoming_stack.push_back(e);
           return;
       }
    }

    // ���� ��������:
    // 1. ������, ��� ����������� ������ ������ - ��
    // 2. �������, �� ����� prompt/������ ������� - ��
    // 3. �������, �� �� ����� �� ������� - ������� �������
    parseData parse_data;
    if (window == 0)
        m_parser.parse(text, text_len, false, &parse_data);
    else
    {
        // ���������� ��������� parser ��� �������������� ����,
        // ����� �� ��������� ������ � ������� ���� (� ������� ���� � ������� �����).
        m_parser2.parse(text, text_len, false, &parse_data);
    }
    
    if (flags & GAME_CMD)
    {
        parseDataStrings& ps = parse_data.strings;
        for (int i = 0, e = ps.size(); i < e; ++i)
            ps[i]->gamecmd = true;
    }

    if (flags & GAME_LOG)
    {
        parseDataStrings& ps = parse_data.strings;
        for (int i = 0, e = ps.size(); i < e; ++i)
            ps[i]->system = true;
    }

#ifdef MARKERS_IN_VIEW       // ��� �������
    parseDataStrings &p = parse_data.strings;
    MARKPROMPTUNDERLINE(p);  // ����� �� prompt
    if (flags & FROM_STACK)  // ������� �� ����� �� ������� ��������� ������
    {
        if (flags & FROM_TIMER)
        {
            int color = 4;
            MudViewString *last = m_pHost->getLastString(0);
            if (last && last->prompt && !last->gamecmd)
                color = 1;
            if (last && !last->prompt && last->gamecmd)
                color = 2;
            if (last && last->prompt && last->gamecmd)
                color = 3;
            MARKINVERSEDCOLOR(p, color);

            if (last) { //todo
                tstring text; last->getText(&text);
                OutputDebugString(text.c_str());
                tchar tmp[16];
                _itow(m_prompt_mode, tmp, 10);
                OutputDebugString(tmp);
                OutputDebugString(L"\r\n");
            }
        }
        else
            MARKINVERSED(p);
    }
    if (!p.empty())          // ������ - ���� ������ �� �������
    {
        MudViewString *s = p[0];
        MudViewStringBlock b;
        if (parse_data.update_prev_string)
            b.string = L"*";
        b.string.append(L"{");
        b.params.text_color = 5;
        b.params.intensive_status = 1;
        s->blocks.insert(s->blocks.begin(), b);
        int last = p.size() - 1;
        s = p[last]; b.string = L"}";
        s->blocks.push_back(b);
    }
#endif

    if (flags & GAME_LOG)
        parse_data.update_prev_string = false;

    // accumulate last string in one
    m_pHost->accLastString(window, &parse_data);

    // ������� ������� ����� �� ���� ������, ���� ��� ������� ������
    if (window == 0 && !(flags & (GAME_LOG | GAME_CMD)))
    {
        if (processStack(parse_data, flags))
            return;
    }

    // collect strings in parse_data in one with same colors params
    ColorsCollector pc;
    pc.process(&parse_data);
    printIncoming(parse_data, flags, window);
}

bool LogicProcessor::processStack(parseData& parse_data, int flags)
{
    // find prompts in parse data (place to insert stack -> last gamecmd/prompt/or '>')
    const int max_lines_without_prompt = 7;
    bool p_exist = false;
    int last_game_cmd = -1;
    bool use_template = propData->recognize_prompt ? true : false;
    for (int i = 0, e = parse_data.strings.size(); i < e; ++i)
    {
        MudViewString *s = parse_data.strings[i];
        if (s->prompt) { p_exist = true; }
        if (s->gamecmd || s->prompt || s->system) { last_game_cmd = i; continue; }
        if (use_template)
        {
            // recognize prompt string via template
            tstring text;  s->getText(&text);
            m_prompt_pcre.find(text);
            if (m_prompt_pcre.getSize())
            {
                s->setPrompt(m_prompt_pcre.getLast(0));
                last_game_cmd = i;
                p_exist = true;
            }
        }
    }

    if (p_exist)
    {
       m_prompt_counter = 0;
       m_prompt_mode = USER;
    }
    else
    {
       if (m_prompt_mode == USER)
       {
           m_prompt_counter += parse_data.strings.size();
           if (m_prompt_counter > max_lines_without_prompt) { m_prompt_mode = UNIVERSAL; m_prompt_counter = 0; }
       }

       // ��� iacga/�������� ������ ������� ����� ����� ������� ���� ����� ������������� ������
       // ����������� ����� ������ �� promt ���� �������
       if (m_prompt_mode == OFF || m_prompt_mode == UNIVERSAL)
       {
           last_game_cmd = -1;
           parseDataStrings tmp;       // ��������� �����           
           for (int i = 0, e = parse_data.strings.size(); i < e; ++i)
           {
               int last = tmp.size();
               MudViewString *s = parse_data.strings[i];
               tmp.push_back(s);
               if (s->gamecmd || s->system) { last_game_cmd = last; continue; }

               tstring text; s->getText(&text);
               m_univ_prompt_pcre.find(text);
               if (m_univ_prompt_pcre.getSize())
               //if (false)
               {
                   int end_prompt = m_univ_prompt_pcre.getLast(0);
                   s->setPrompt(end_prompt);
                   last_game_cmd = last;
                   p_exist = true;

                   tstring after_prompt(text.substr(end_prompt));
                   tstring_trim(&after_prompt);
                   if (!after_prompt.empty())
                   {
                       MudViewString *s2 = s->divideString(end_prompt);
                       if (!s2->blocks.empty()) tstring_trimleft(&s2->blocks[0].string);
#ifdef MARKERS_IN_VIEW
                       s2->prompt = -1;
#endif
                       tmp.push_back(s2);
                   }
               }
           }
           parse_data.strings.swap(tmp);

           if (p_exist)
           {
               m_prompt_mode = UNIVERSAL;
               m_prompt_counter = 0;
           }
           else
           {
               m_prompt_counter += parse_data.strings.size();
               if (m_prompt_counter > max_lines_without_prompt) { m_prompt_mode = OFF; m_prompt_counter = 0; }
           }
       }
    }

    if (m_incoming_stack.empty()) // ������ ��������� �������, ��. ��������� �������� ������� prompt � �������
        return false;
    if (last_game_cmd == -1)      // ��� ����� ��� ������� ������ �� �����
        return false;

    // div current parseData at 2 parts
    parseData pd;
    pd.update_prev_string = parse_data.update_prev_string;
    pd.strings.assign(parse_data.strings.begin(), parse_data.strings.begin() + last_game_cmd + 1);
    MARKITALIC(pd.strings);     // ����� �������
    printIncoming(pd, flags, 0);
    pd.strings.clear();

    // insert stack between 2 parts
    printStack();

    pd.update_prev_string = false;
    pd.strings.assign(parse_data.strings.begin() + last_game_cmd + 1, parse_data.strings.end());
    MARKBLINK(pd.strings);      // ����� �������
    printIncoming(pd, flags, 0);
    pd.strings.clear();
    parse_data.strings.clear();
    return true;
}

void LogicProcessor::printStack(int flags)
{
    if (m_incoming_stack.empty())
        return;
    for (int i = 0, e = m_incoming_stack.size(); i < e; ++i)
    {
        const stack_el &s = m_incoming_stack[i];
        const tstring &t = s.text;
        processIncoming(t.c_str(), t.length(), s.flags | FROM_STACK | flags);
    }
    m_incoming_stack.clear();
}

void LogicProcessor::printIncoming(parseData& parse_data, int flags, int window)
{
    if (parse_data.strings.empty())
        return;

    if (!m_connected)
        flags |= SKIP_ACTIONS;

    // final step for data
    // preprocess data via plugins
    if (!(flags & SKIP_PLUGINS))
        m_pHost->preprocessText(window, &parse_data);

    // array for new cmds from actions
    std::vector<tstring> new_cmds;
    if (!(flags & SKIP_ACTIONS))
        m_helper.processActions(&parse_data, &new_cmds);

    if (!(flags & SKIP_SUBS))
    {
        m_helper.processAntiSubs(&parse_data);
        m_helper.processGags(&parse_data);
        m_helper.processSubs(&parse_data);
    }

    if (!(flags & SKIP_HIGHLIGHTS))
        m_helper.processHighlights(&parse_data);

    // postprocess data via plugins
    if (!(flags & SKIP_PLUGINS))
        m_pHost->postprocessText(window, &parse_data);

    int log = m_wlogs[window];
    if (log != -1)
        m_logs.writeLog(log, parse_data);     // write log
    m_pHost->addText(window, &parse_data);    // send processed text to view

    for (int i = 0, e = new_cmds.size(); i < e; ++i) // process actions' result
        processCommand(new_cmds[i]);
}

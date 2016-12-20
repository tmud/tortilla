#include "stdafx.h"
#include "accessors.h"
#include "logicProcessor.h"

void LogicProcessor::processStackTick()
{
    processQueueCommand();
    if (!m_plugins_log_cache.empty())
    {
        PropertiesData *pdata = tortilla::getProperties();
        if (!pdata->plugins_logs)
        {
            m_plugins_log_cache.clear();
            return;
        }
        int window = pdata->plugins_logs_window;
        MudViewString *last = m_pHost->getLastString(window);
        if (last && !last->prompt && !last->gamecmd && !last->system && !last->triggered)
            { /*skip*/ }
        else
        {
            m_plugins_log_blocked = true;
            std::vector<tstring> tmp;
            tmp.swap(m_plugins_log_cache);
            for (int i=0,e=tmp.size(); i<e; ++i){
                tstring &t = tmp[i];
                processIncoming(t.c_str(), t.length(), SKIP_ACTIONS|SKIP_SUBS|GAME_LOG/*|SKIP_PLUGINS*/, window);
            }
            tmp.clear();
            m_plugins_log_blocked = false;
        }
    }

    if (!m_plugins_log_toblocked.empty() && m_plugins_log_cache.empty())
    {
        m_plugins_log_toblocked.swap(m_plugins_log_cache);
    }

    if (!m_connected)
    {
        printStack(FROM_TIMER|FROM_STACK);
        return;
    }

    if (m_prompt_mode == OFF)
        return;
    MudViewString *last = m_pHost->getLastString(0);
    if (last && !last->prompt && !last->gamecmd && !last->system && !last->triggered)
            return;
    printStack(FROM_TIMER|FROM_STACK);
}
 
void LogicProcessor::processIncoming(const WCHAR* text, int text_len, int flags, int window)
{
    if (window == 0 && m_prompt_mode != OFF && (flags & (GAME_LOG | GAME_CMD)) && !(flags & FROM_STACK))
    {
       MudViewString *last = m_pHost->getLastString(0);
       if (last && !last->prompt && !last->gamecmd && !last->system && !last->triggered)
       {
           // в стек, если нельзя сразу добавить команды в окно (нет prompt/gamecmd, возможно это разрыв текста).
           stack_el e;
           e.text.assign(text, text_len);
           e.flags = flags;
           m_incoming_stack.push_back(e);
           return;
       }
    }

    // сюда попадаем:
    // 1. данные, как продолжение старых данных - ок
    // 2. команды, но после prompt/другой команды - ок
    // 3. команды, но из стека по таймеру - попытка вставки
    parseData parse_data;
    if (window == 0 && !(flags & GAME_LOG) && !(flags & FROM_STACK))
    {
        MudViewParserOscPalette palette;
        m_parser.parse(text, text_len, true, &parse_data, &palette);

        // Работа с OSC палитрой
        if (palette.reset_colors)
            m_pHost->resetOscColors();
        else if (!palette.colors.empty())
        {
            MudViewParserOscPalette::colors_iterator it  = palette.colors.begin(), it_end = palette.colors.end();
            for (;it!=it_end;++it)
                m_pHost->setOscColor(it->first, it->second);
        }
    }
    else
    {
        // используем отдельный parser для дополнительных окон,
        // чтобы не сбивались данные в главном окне (в парсере инфа о прошлом блоке).
        m_parser2.parse(text, text_len, false, &parse_data, NULL);
    }

    if (flags & GAME_CMD)
    {
        parseDataStrings& ps = parse_data.strings;
        for (int i = 0, e = ps.size(); i < e; ++i)
            ps[i]->gamecmd = true;
        parse_data.update_prev_string = true;
    }

    if (flags & GAME_LOG)
    {
        parseDataStrings& ps = parse_data.strings;
        for (int i = 0, e = ps.size(); i < e; ++i)
            ps[i]->system = true;
        parse_data.update_prev_string = false;
    }

    if (flags & NEW_LINE)
    {
        parse_data.update_prev_string = false;
    }

#ifdef MARKERS_IN_VIEW       // для отладки
    parseDataStrings &p = parse_data.strings;
    MARKPROMPTUNDERLINE(p);  // метка на prompt
    if (flags & FROM_STACK)  // команды из стека по таймеру отдельным цветом
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
        }
        else
            MARKINVERSED(p);
    }
    if (!p.empty())          // скобки - блок текста от сервера
    {
        MudViewString *s = p[0];
        MudViewStringBlock b;
        if (parse_data.update_prev_string)
            b.string = L"+";
        b.string.append(L"{");
        b.params.text_color = 5;
        b.params.intensive_status = 1;
        s->blocks.insert(s->blocks.begin(), b);
        int last = p.size() - 1;
        s = p[last]; b.string = L"}";
        if (parse_data.last_finished)
            b.string.append(L".");
        s->blocks.push_back(b);
    }
#endif

    if (window == 0 && (flags & (GAME_CMD|GAME_LOG)) && !(flags & FROM_STACK))
    {
        MudViewString *last = m_pHost->getLastString(0);
        if (last && (last->prompt || last->system || last->gamecmd) && !m_incoming_stack.empty())
        {
            printStack();
        }
    }

    m_pHost->accLastString(window, &parse_data);

    // попытка вставки стека по ходу данных, если это обычные данные
    if (window == 0 && !(flags & (GAME_LOG | GAME_CMD)))
    {
        MudViewString *last = m_pHost->getLastString(0);
        if (last && (last->prompt || last->system || last->gamecmd) && !m_incoming_stack.empty())
        {
            printStack();
        }
        else
        {
            if (processStack(parse_data, flags))
                return;
        }
    }

    // collect strings in parse_data in one with same colors params
    if (!(flags & GAME_CMD))
    {
        ColorsCollector pc;
        pc.process(&parse_data.strings);
    }
    printIncoming(parse_data, flags, window);
}

bool LogicProcessor::processStack(parseData& parse_data, int flags)
{
    class LastGameCmd
    {
    public:
        int index;
        LastGameCmd() : index(-1) {}
        void set(int new_index) { if (index == -1) index = new_index; }
        void reset() { index = -1; }
    } last_game_cmd;


    // find prompts in parse data (place to insert stack -> last gamecmd/prompt/or '>')
    const int max_lines_without_prompt = 30;
    bool p_exist = false;
    PropertiesData *pdata = tortilla::getProperties();
    for (int i = 0, e = parse_data.strings.size(); i < e; ++i)
    {
        MudViewString *s = parse_data.strings[i];
        if (s->prompt) {
            m_prompt_counter = 0;
            m_prompt_mode = IACGA;
            p_exist = true; 
        }
        if (s->gamecmd || s->prompt || s->system || s->triggered) {
            last_game_cmd.set(i);
            continue; 
        }
        if (pdata->recognize_prompt)
        {
            // recognize prompt string via template
            tstring text;  s->getText(&text);
            m_prompt_pcre.find(text);
            if (m_prompt_pcre.getSize())
            {
                s->setPrompt(m_prompt_pcre.getLast(0));
                last_game_cmd.set(i);
                m_prompt_counter = 0;
                m_prompt_mode = USER;
                p_exist = true;
            }
        }
    }

    if (!p_exist)
    {
       if (m_prompt_mode == USER || m_prompt_mode == IACGA)
       {
           m_prompt_counter += parse_data.strings.size();
           if (m_prompt_counter > max_lines_without_prompt) {
               m_prompt_mode = OFF; m_prompt_counter = 0; }
       }

       // без iacga/заданный шаблон пробуем найти место вставки сами через универсальный шаблон
       // параллельно делим строку по prompt если находим
       if (m_prompt_mode == OFF || m_prompt_mode == UNIVERSAL)
       {
           last_game_cmd.reset();
           parseDataStrings tmp;       // временный буфер
           for (int i = 0, e = parse_data.strings.size(); i < e; ++i)
           {
               int last = tmp.size();
               MudViewString *s = parse_data.strings[i];
               tmp.push_back(s);
               if (s->gamecmd || s->system || s->triggered) {
                   last_game_cmd.set(last);
                   continue;
               }

               tstring text; s->getText(&text);
               m_univ_prompt_pcre.find(text);
               if (m_univ_prompt_pcre.getSize())
               //if (false)
               {
                   int end_prompt = m_univ_prompt_pcre.getLast(0);
                   s->setPrompt(end_prompt);
                   last_game_cmd.set(last);
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
               if (m_prompt_counter > max_lines_without_prompt) {
                   m_prompt_mode = OFF; m_prompt_counter = 0; }
           }
       }
    }

    if (m_incoming_stack.empty())   // нельзя поставить вначале, тк. требуется контроль наличия prompt в трафике
        return false;
    if (last_game_cmd.index == -1)  // нет места для вставки данных из стека
        return false;

    // div current parseData at 2 parts
    parseData pd;
    pd.update_prev_string = parse_data.update_prev_string;
    pd.last_finished = true;
    pd.strings.assign(parse_data.strings.begin(), parse_data.strings.begin() + last_game_cmd.index + 1);
    MARKITALIC(pd.strings);     // режим отладки
    printIncoming(pd, flags, 0);
    pd.strings.clear();

    // insert stack between 2 parts
    printStack();

    pd.update_prev_string = false;
    pd.last_finished = parse_data.last_finished;
    pd.strings.assign(parse_data.strings.begin() + last_game_cmd.index + 1, parse_data.strings.end());
    MARKBLINK(pd.strings);      // режим отладки
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
        processIncoming(t.c_str(), t.length(), s.flags | FROM_STACK | flags, 0);
    }
    m_incoming_stack.clear();
}

void LogicProcessor::printIncoming(parseData& parse_data, int flags, int window)
{
    if (parse_data.strings.empty())
        return;
    if (!m_connected && !(flags & WORK_OFFLINE))
        flags = flags | SKIP_ACTIONS | SKIP_SUBS;

    parseDataStrings &pds = parse_data.strings;
    if (parse_data.update_prev_string)
    {
        MudViewString *s = parse_data.strings[0];
        if (s->prompt && s->gamecmd)
        {
            pds.erase(pds.begin());
            parseData pd;
            pd.update_prev_string = true;
            pd.last_finished = true;
            pd.strings.push_back(s);
            pipelineParseData(pd, flags | SKIP_ACTIONS | SKIP_HIGHLIGHTS | SKIP_SUBS, window);
            pd.strings.clear();
        }
    }

    if (pds.empty())
        return;

    if (window == 0)
    {
        int last = pds.size() - 1;
        MudViewString *s = pds[last];
        if (!s->prompt && !s->gamecmd && !s->system && !s->triggered)
        {
            // last string not finished (игровой текст, не промпт, не команда и не лог)
            parse_data.last_finished = false;
#ifdef MARKERS_IN_VIEW
            std::vector<MudViewStringBlock> &b = s->blocks;
            for (int i = 0, e = b.size(); i < e; ++i)
                b[i].params.blink_status = 1;
#endif
        }
    }
    pipelineParseData(parse_data, flags, window);
}

void LogicProcessor::pipelineParseData(parseData& parse_data, int flags, int window)
{
    LogicPipelineElement *e = m_pipeline.createElement();
    printParseData(parse_data, flags, window, e);
    while (!e->triggers.empty() || !e->commands.empty())
    {
        if (!e->triggers.empty())
        {
            // выполняем функции lua триггеров
            for (int i=0,si=e->triggers.size();i<si;++i)
                e->triggers[i]->run();
        }
        if (!e->commands.empty())
        {
            // выполняем команды actions триггеров
            runCommands(e->commands);
        }
        LogicPipelineElement *e2 = m_pipeline.createElement();
        printParseData(e->data, flags|SKIP_PLUGINS_BEFORE, window, e2);
        m_pipeline.freeElement(e);
        e = e2;
    }
    m_pipeline.freeElement(e);
    m_pHost->clearDropped(window);
}

void LogicProcessor::printParseData(parseData& parse_data, int flags, int window, LogicPipelineElement *pe)
{
    PropertiesData *pdata = tortilla::getProperties();
    const PropertiesData::working_mode &m = pdata->mode;
    if (!m.actions)
        flags |= SKIP_ACTIONS;
    if (!m.antisubs)
        flags |= SKIP_COMPONENT_ANTISUBS;
    if (!m.gags)
        flags |= SKIP_COMPONENT_GAGS;
    if (!m.subs)
        flags |= SKIP_COMPONENT_SUBS;
    if (!m.highlights)
        flags |= SKIP_HIGHLIGHTS;
    if (!m.plugins)
        flags |= SKIP_COMPONENT_PLUGINS;

    // save all logs from plugins in cache (to break cycle before/after -> log -> befor/after -> app crash)
    m_plugins_log_tocache = true;

    // final step for data
    // preprocess data via plugins
    if (!(flags & SKIP_PLUGINS_BEFORE))
        m_pHost->preprocessText(window, &parse_data);

    if (!(flags & SKIP_SUBS))
    {
        if (!(flags & SKIP_COMPONENT_ANTISUBS))
            m_helper.processAntiSubs(&parse_data);
        if (!(flags & SKIP_COMPONENT_GAGS))
            m_helper.processGags(&parse_data);
        if (!(flags & SKIP_COMPONENT_SUBS))
            m_helper.processSubs(&parse_data);
    }

    // process lua triggers and actions
    PluginsTriggersHandler *luatriggers = m_pHost->getPluginsTriggers();
    bool skip_actions = (flags & SKIP_ACTIONS);
    for (int j=0,je=parse_data.strings.size()-1; j<=je; ++j)
    {
        bool triggered = false;
        if (!skip_actions) {
            if (!(flags & SKIP_COMPONENT_PLUGINS))
              triggered = luatriggers->processTriggers(parse_data, j, pe);
        }
        bool actions = false;
        if (!skip_actions) {
            actions = m_helper.processActions(&parse_data, j, pe);
        }
        if (triggered || actions)
        {
            MudViewString *s = parse_data.strings[j];
            s->triggered = true; //чтобы команда могла напечататься сразу после строчки на которую сработал триггер
            parseData &not_processed = pe->data;
            not_processed.last_finished = parse_data.last_finished;
            parse_data.last_finished = true;
            not_processed.update_prev_string = false;
            int from = j + 1;
            not_processed.strings.assign(parse_data.strings.begin() + from, parse_data.strings.end());
            parse_data.strings.resize(from);
            break;
        }
    }

    if (!(flags & SKIP_HIGHLIGHTS))
        m_helper.processHighlights(&parse_data);

    // postprocess data via plugins
    if (!(flags & SKIP_PLUGINS_AFTER))
        m_pHost->postprocessText(window, &parse_data);

    m_plugins_log_tocache = false;

    int log = m_wlogs[window];
    if (log != -1)
        m_logs.writeLog(log, parse_data);     // write log
    m_pHost->addText(window, &parse_data);    // send processed text to view
}

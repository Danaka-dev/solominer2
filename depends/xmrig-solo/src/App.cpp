/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2018      Lee Clagett <https://github.com/vtnerd>
 * Copyright 2018-2020 SChernykh   <https://github.com/SChernykh>
 * Copyright 2016-2020 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 * Copyright 2023-2024 The solominer developers
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

//////////////////////////////////////////////////////////////////////////////
#include "App.h"
#include "backend/cpu/Cpu.h"
#include "base/io/Console.h"
#include "base/io/log/Log.h"
#include "base/io/log/Tags.h"
#include "base/io/Signals.h"
#include "base/kernel/Platform.h"
#include "core/config/Config.h"
#include "core/Controller.h"
#include "Summary.h"

#include <uv.h>

//////////////////////////////////////////////////////////////////////////////
xmrig::App::App(Process *process) : m_loop(nullptr)
{
    m_controller = std::make_shared<Controller>(process);
}

xmrig::App::~App()
{
    Cpu::release();
}

//////////////////////////////////////////////////////////////////////////////
void xmrig::App::Main() {
    uv_loop_t *loop = uv_default_loop();

    uv_run( loop ,UV_RUN_DEFAULT );
    uv_loop_close(loop);
}

///--
int xmrig::App::Exec( IStrategyListener *strategyListener )
{
    if (!m_controller->isReady()) {
        LOG_EMERG("no valid configuration found , try https://xmrig.com/wizard");

        return 2;
    }

    m_signals = std::make_shared<Signals>(this);

    int rc = 0;
    if (background(rc)) {
        return rc;
    }

    rc = m_controller->init( strategyListener );
    if (rc != 0) {
        return rc;
    }

    if (!m_controller->isBackground()) {
        m_console = std::make_shared<Console>(this);
    }

    Summary::print(m_controller.get());

    if (m_controller->config()->isDryRun()) {
        LOG_NOTICE("%s " WHITE_BOLD("OK"), Tags::config());

        return 0;
    }

    m_controller->start();

///--
    /* m_running = true;

    while( m_running ) {
        usleep(50);
    } */

    uv_loop_t *loop = uv_default_loop();

    m_loop = (void*) loop;

    rc = uv_run( loop ,UV_RUN_DEFAULT );
    uv_loop_close(loop);

    close();

    return rc;
}

void xmrig::App::Stop() {
    m_running = false;

    auto *loop = (uv_loop_t*) m_loop;

    loop->stop_flag++;
}

void xmrig::App::Quit() {
    close();
}

void xmrig::App::doCommand( char cmd ) {
    if( cmd == 3 ) {
        LOG_WARN( "%s " YELLOW("Ctrl+C received, exiting") ,Tags::signal() );
        Quit();
    }
    else {
        m_controller->execCommand(cmd);
    }
};

//////////////////////////////////////////////////////////////////////////////
void xmrig::App::onConsoleCommand( char command ) {
    doCommand(command);
}

void xmrig::App::onSignal( int signum ) {
    switch (signum)
    {
    case SIGHUP:
    case SIGTERM:
    case SIGINT:
        return close();

    default:
        break;
    }
}

void xmrig::App::close() {
    m_signals.reset();
    m_console.reset();

    m_controller->stop();

    Log::destroy();
}

//////////////////////////////////////////////////////////////////////////////
//EOF
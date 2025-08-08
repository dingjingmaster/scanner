//
// Created by dingjing on 2/17/25.
//

#include "pf/policy-filter.h"

#include "pf/qlog.h"
#include "pf/glog.h"
#include "tika-wrap/src/java-env.h"


int main (int argc, char* argv[])
{
    g_log_set_writer_func(c_glog_handler, nullptr, nullptr);
    qInstallMessageHandler(c_qlog_handler);
    sigaction(SIGSEGV, nullptr, nullptr);
    PolicyFilter app(argc, argv);

    app.start();

    return app.exec();
}




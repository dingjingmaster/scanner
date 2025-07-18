//
// Created by dingjing on 2/17/25.
//

#include "pf/policy-filter.h"

#include "pf/qlog.h"
#include "pf/glog.h"


int main (int argc, char* argv[])
{
    g_log_set_writer_func(c_glog_handler, nullptr, nullptr);
    qInstallMessageHandler(c_qlog_handler);
    PolicyFilter app(argc, argv);

    app.start();

    return PolicyFilter::exec();
}




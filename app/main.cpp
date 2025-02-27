//
// Created by dingjing on 2/17/25.
//

#include "pf/policy-filter.h"

#define C_LOG_TAG "AndsecScanner"
void c_qlog_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main (int argc, char* argv[])
{
    qInstallMessageHandler(c_qlog_handler);
    PolicyFilter app(argc, argv);

    app.start();

    return PolicyFilter::exec();
}

void c_qlog_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    printf("[%s", C_LOG_TAG);
    printf(" %s %d %s] ", file, context.line, function);
    printf("%s\n", msg.toUtf8().constData());
}


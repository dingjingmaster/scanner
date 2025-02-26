//
// Created by dingjing on 2/26/25.
//

#ifndef andsec_scanner_POLICY_FILTER_H
#define andsec_scanner_POLICY_FILTER_H
#include <QCoreApplication>


class PolicyFilter : public QCoreApplication
{
    Q_OBJECT
public:
    explicit PolicyFilter(int argc, char** argv);

};



#endif // andsec_scanner_POLICY_FILTER_H

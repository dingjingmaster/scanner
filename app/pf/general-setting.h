//
// Created by dingjing on 25-6-24.
//

#ifndef andsec_scanner_GENERAL_SETTING_H
#define andsec_scanner_GENERAL_SETTING_H
#include <QString>



class GeneralSetting
{
public:
    static GeneralSetting& getInstance();
    qint64 getUploadContextLen() const;
    void setUploadContextLen(qint64 size);

private:
    GeneralSetting();
    GeneralSetting(const GeneralSetting&);
    void operator=(GeneralSetting&) const;
    qint64                      mUploadContextLen = 1024;
    static GeneralSetting       gInstance;
};



#endif // andsec_scanner_GENERAL_SETTING_H

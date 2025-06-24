//
// Created by dingjing on 25-6-24.
//

#include "general-setting.h"


GeneralSetting GeneralSetting::gInstance;

GeneralSetting& GeneralSetting::getInstance()
{
    return gInstance;
}

qint64 GeneralSetting::getUploadContextLen() const
{
    return mUploadContextLen;
}

void GeneralSetting::setUploadContextLen(qint64 size)
{
    mUploadContextLen = size;
}

GeneralSetting::GeneralSetting()
{
}

GeneralSetting::GeneralSetting(const GeneralSetting&)
{
}

void GeneralSetting::operator=(GeneralSetting&) const
{
}

#include "GroupData.h"

namespace ocr_orc {

void GroupData::addRegion(const QString& regionName) {
    if (!contains(regionName)) {
        regionNames.append(regionName);
    }
}

void GroupData::removeRegion(const QString& regionName) {
    regionNames.removeAll(regionName);
}

bool GroupData::contains(const QString& regionName) const {
    return regionNames.contains(regionName);
}

int GroupData::count() const {
    return regionNames.size();
}

bool GroupData::isEmpty() const {
    return regionNames.isEmpty();
}

} // namespace ocr_orc


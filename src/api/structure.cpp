#include "gridpp.h"

gridpp::StructureFunction::StructureFunction(float localization_distance) {
    if(!gridpp::util::is_valid(localization_distance) || localization_distance < 0)
        throw std::invalid_argument("Invalid 'localization_distance' in structure");

    mLocalizationDistance = localization_distance;
}
float gridpp::StructureFunction::barnes_rho(float dist, float length) const {
    if(!gridpp::util::is_valid(length) || length == 0)
        // Disabled
        return 1;
    if(!gridpp::util::is_valid(dist))
        return 0;
    float v = dist / length;
    return exp(-0.5 * v * v);
}
float gridpp::StructureFunction::cressman_rho(float dist, float length) const {
    if(!gridpp::util::is_valid(length) || length == 0)
        // Disabled
        return 1;
    if(!gridpp::util::is_valid(dist))
        return 0;
    return (length * length - dist * dist) / (length * length + dist * dist);
}
float gridpp::StructureFunction::localization_distance() const {
    return mLocalizationDistance;
}

/** Barnes */
gridpp::BarnesStructure::BarnesStructure(float h, float v, float w, float hmax) :
    gridpp::StructureFunction(h) {
    if(hmax < 0) {
        throw std::invalid_argument("hmax must be >= 0");
    }
    if(gridpp::util::is_valid(hmax))
        mLocalizationDistance = hmax;
    else {
        // Calculate the horizontal localization radius. For min_rho=0.0013, the factor is 3.64
        float default_min_rho = 0.0013;
        mLocalizationDistance = sqrt(-2*log(default_min_rho)) * h;
    }
    mH = h;
    mV = v;
    mW = w;
}
float gridpp::BarnesStructure::corr(const Point& p1, const Point& p2) const {
    float hdist = gridpp::KDTree::calc_distance_fast(p1.lat, p1.lon, p2.lat, p2.lon);
    if(hdist > localization_distance())
        return 0;
    float rho = gridpp::StructureFunction::barnes_rho(hdist, mH);
    if(gridpp::util::is_valid(p1.elev) && gridpp::util::is_valid(p2.elev)) {
        float vdist = p1.elev - p2.elev;
        rho *= gridpp::StructureFunction::barnes_rho(vdist, mV);
    }
    if(gridpp::util::is_valid(p1.laf) && gridpp::util::is_valid(p2.laf)) {
        float lafdist = p1.laf - p2.laf;
        rho *= gridpp::StructureFunction::barnes_rho(lafdist, mW);
    }
    return rho;
}
gridpp::StructureFunction* gridpp::BarnesStructure::clone() const {
    gridpp::StructureFunction* val = new gridpp::BarnesStructure(mH, mV, mW, mLocalizationDistance);
    return val;
}

/** Cressman */
gridpp::CressmanStructure::CressmanStructure(float h, float v, float w) :
    gridpp::StructureFunction(h) {
    mH = h;
    mV = v;
    mW = w;
}
float gridpp::CressmanStructure::corr(const Point& p1, const Point& p2) const {
    float hdist = gridpp::KDTree::calc_distance_fast(p1.lat, p1.lon, p2.lat, p2.lon);
    float rho = gridpp::StructureFunction::cressman_rho(hdist, mH);
    if(gridpp::util::is_valid(p1.elev) && gridpp::util::is_valid(p2.elev)) {
        float vdist = p1.elev - p2.elev;
        rho *= gridpp::StructureFunction::cressman_rho(vdist, mV);
    }
    if(gridpp::util::is_valid(p1.laf) && gridpp::util::is_valid(p2.laf)) {
        float lafdist = p1.laf - p2.laf;
        rho *= gridpp::StructureFunction::cressman_rho(lafdist, mW);
    }
    return rho;
}
gridpp::StructureFunction* gridpp::CressmanStructure::clone() const {
    gridpp::StructureFunction* val = new gridpp::CressmanStructure(mH, mV, mW);
    return val;
}

/** CrossValidation */
gridpp::CrossValidation::CrossValidation(StructureFunction& structure, float dist) :
        StructureFunction(0){
    mLocalizationDistance = structure.localization_distance();
    m_structure = structure.clone();
    m_dist = dist;
}
float gridpp::CrossValidation::corr(const Point& p1, const Point& p2) const {
    float hdist = gridpp::KDTree::calc_distance_fast(p1.lat, p1.lon, p2.lat, p2.lon);
    if(hdist <= m_dist)
        return 0;
    return m_structure->corr(p1, p2);
}
gridpp::StructureFunction* gridpp::CrossValidation::clone() const {
    gridpp::StructureFunction* val = new gridpp::CrossValidation(*m_structure);
    return val;
}


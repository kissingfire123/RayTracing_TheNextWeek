#ifndef _MOVING_SPHERE_H_
#define _MOVING_SPHERE_H_

#include "hitable.h"

class lambertian;

// compare with sphere, add center1,center2, and time1, time2
class movingSphere :public hitable {
public:
    movingSphere() {}
    explicit movingSphere(point3 cen1, point3 cen2,
        double time1,double time2,
        double r, const std::shared_ptr<material>& material = nullptr) :
        center1_(cen1),
        center2_(cen2),
        time1_(time1),
        time2_(time2),
        radius_(r),
        material_(material) {}
    virtual bool hit(const ray& r, float tmin, float tmax, hit_record& rec)const override;
    point3 getCenter(double time) const;
private:
    point3 center1_;
    point3 center2_;
    double time1_ = 0.f;
    double time2_ = 0.f;
    double radius_ = 0.f;
    shared_ptr<material> material_;
};

// linear distribution
point3 movingSphere::getCenter(double time) const{
    return center1_ + ((time - time1_) / (time2_ - time1_)) *(center2_ - center1_);
}

bool movingSphere::hit(const ray& r, float tmin, float tmax, hit_record& reco) const {
    // 计算delta
    vec3 oc = r.origin() - getCenter(r.time());// Motion blur:concern time element
    float a = dot(r.direction(), r.direction());
    float half_b = dot(oc, r.direction());//简化运算,统一除以2
    float c = dot(oc, oc) - radius_ * radius_;
    float discriminant = half_b * half_b - a * c;
    auto has_resolve_t = [&](float t)->bool {
        if (t <= tmax && t >= tmin) {
            reco.t_ = t;
            reco.p_ = r.at_Parameter(reco.t_);
            vec3 outward_normal = (reco.p_ - getCenter(r.time())) / radius_;
            reco.setFaceNormal(r, outward_normal);
            reco.mate_ptr = material_.get();
            return true;
        }
        return false;
    };
    if (discriminant < 0) {
        return false;//方程无解, 光线与球没有交点
    }
    auto sqrtd = sqrt(discriminant);

    bool hitable = has_resolve_t((-half_b - sqrtd) / a);//优先看近点
    if (hitable) return true;
    hitable = has_resolve_t((-half_b + sqrtd) / a);
    if (hitable) return true;

    return false;
}


#endif
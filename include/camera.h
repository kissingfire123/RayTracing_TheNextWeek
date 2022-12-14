#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "ray.h"

class camera{
public:
    //Ch1~Ch9,call this
    explicit camera(){}

    //since Ch10,call this
    // verticalFov:角度制
    // focus_dis:焦距,from--> at 的间距,默认为1.0
    explicit camera(float verticalFov, float aspect,
        vec3 lookFrom,vec3 lookAt,vec3 lookUp,//Ch10:建立camera坐标系
        float apeture = 0.0,float focus_dis = 1.0)//Ch11:散焦,景深现象,需要设置光圈和焦长 
           :origin_(lookFrom),lens_radius_(apeture/2){ 
        float theta = verticalFov * RTW_PI / 180;
        float half_height = tan(theta/2);
        float half_width = aspect * half_height;

        //camera的uvw坐标系,w相当于之前的z,且-z指向camera
        w_ = unit_vector(lookFrom - lookAt);
        u_ = unit_vector(cross(lookUp,w_));
        v_ = cross(w_,u_);

        lower_left_corner_ =  origin_ - (half_width*u_ + half_height*v_ + w_)*focus_dis;
        horizontal_ = 2 * half_width  * u_ * focus_dis;
        vertical_   = 2 * half_height * v_ * focus_dis;
    }

    void set_time_interval(float t1, float t2) {
        t1_ = t1;
        t2_ = t2;
    }

    // for antiliasing,move the origin,simu multi-rays for same one pixel 
    ray get_ray(float s, float t){
        vec3 rd = lens_radius_ * camera::random_unit_disk();
        vec3 offset = u_ * rd.x() + v_ * rd.y();
        vec3 offsetOrigin = origin_ + offset;//Ch11之前,offset都为0,即没有散焦
        return ray(offsetOrigin, 
            lower_left_corner_ + s * horizontal_ + t * vertical_ - offsetOrigin,
            random_double(t1_,t2_));//return a random ray[t1,t2] 
    }

    // 为了模拟散焦现象,将点光源扩散成区域圆盘光源(x,y,0),其中x,y均[-1,1]
    static vec3 random_unit_disk() {
        vec3 originP;
        do {
            originP = 2.0 * vec3(random_double(), random_double(),0) - vec3(1,1,0);
        } while (originP.length() >= 1.0);
        return originP;
    }

private:
    vec3 origin_ = vec3(0.0, 0.0, 0.0);
    vec3 lower_left_corner_ = vec3(-2.0, -1.0 , -1.0);
    vec3 horizontal_ = vec3(4.0 , 0.0, 0.0);
    vec3 vertical_ = vec3(0.0,2.0,0.0);

    //since Ch10 ,have attrs below
    //camera的坐标系,观察点为远点,w相当于之前的z,且-z指向camera
    vec3 u_ = vec3(0.0, 0.0, 0.0); 
    vec3 v_ = vec3(0.0, 0.0, 0.0);  
    vec3 w_ = vec3(0.0, 0.0, 0.0); 
    float lens_radius_ = 0.0;//模拟光圈尺寸,可以控制散焦的程度
    float t1_ = 0.f;// time start
    float t2_ = 0.f;// time close
};



#endif

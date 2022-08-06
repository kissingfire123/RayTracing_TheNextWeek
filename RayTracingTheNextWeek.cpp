#include <iostream>
#include <fstream>
#include <functional>
#if    RTW_OS_WIN //通过CMake自行定义,区分系统类型
#include <io.h>
#elif  RTW_OS_MAC
#include <unistd.h>
#endif
#include <chrono>
#include <vector>
#include <thread>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"//vscode插件(PPM/PGM Viewer)可打开*.ppm,但不如*.bmp方便,都输出
#include "progress.h"

#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "movingSphere.h"
#include "hitable_list.h"
#include "camera.h"
#include "material.h"


//Declaration
int Ch1_MotionBlurScene(std::string imgFilePath);


int main(int argc, char* argv[]) { 
#if  RTW_OS_WIN //have no idea for Xcode
#ifdef _DEBUG
    std::cout << "Now running model is Debug\n";  // debug enable
#else
    std::cout << "Now running model is Release\n";// more faster
#endif
#endif
    //以下每个Chx...函数都可以单独运行,互不影响,可以屏蔽其中任意几个单独运行其他的 
    Ch1_MotionBlurScene("./Image1_MotionBlurScene.ppm");
    return 0;
}


int Ch1_MotionBlurScene(std::string imgFilePath) {
    RtwProgress rtwProgress(imgFilePath, g_Height);
    double blurTime1 = 0.0 , blurTime2 = 1.0;
    //Ch8: modify getColor()
    using getColorFuncType = std::function<vec3(const ray&r, hitable *world, int depth)>;
    getColorFuncType getColor = [&](const ray&r, hitable *world, int depth) -> vec3 {
        hit_record reco;
        if (depth > g_DepthThreshold) {
            return color(0, 0, 0);
        }
        //Ch6:根据光线击中的最近点,进行渲染着色
        if (world->hit(r, 0.001, g_MAX_TmFloat, reco)) {//Ch7: 0.001f,表示去除靠近0的浮点值,避免浮点精度带来的毛刺
            ray scattered;
            vec3 attenuation;//Ch8 : 材料属性,反射率,吸光率
            if (reco.mate_ptr->scatter(r, reco, attenuation, scattered)) {
                return attenuation * getColor(scattered, world, depth + 1);//递归,继续反射
            }
            return color(0, 0, 0);
        }

        vec3 unit_direction = unit_vector(r.direction());
        float t = 0.5 * (unit_direction.y() + 1.0);
        return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0);
    };

    std::vector<shared_ptr<hitable>> hitableList;
    auto getRandomWorldScene = [&]()-> shared_ptr<hitable>{
        int subjectNum = 500;
        int worldEdge = 11;// 22*22=484<500,make sure the valid range of hitableList
        // list[0]:the platform floor of the scene
        hitableList.push_back(make_shared<sphere>(vec3(0,-1000,0),1000,std::make_shared<lambertian>(vec3(0.5,0.5,0.5))));
        int idx = 1;//index of sphere subjects
        // hundred of little spheres
        for (int a = -worldEdge; a < worldEdge; ++a) {
            for (int b = -worldEdge; b < worldEdge; ++b) {
                float choose_material = random_double();
                vec3 center(a+0.9*random_double(),0.2, b + 0.9 * random_double());
                if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
                    if (choose_material < 0.8) { //diffuse,粗糙面
                        vec3 randomColor = vec3(random_double(), random_double(), random_double());
                        auto diffuseMaterial = make_shared<lambertian>(randomColor);
                        // add blur sphere
                        point3 center2 = center + (vec3(0, random_double(0, 0.5), 0));
                        hitableList.push_back(make_shared<movingSphere>(center,center2, blurTime1, blurTime2,0.2, diffuseMaterial));
                    }
                    else if (choose_material < 0.95) {//metal,金属表面
                        vec3 randomColor = vec3(0.5*(1+random_double()), 0.5*(1 + random_double()), 0.5*(1 + random_double()));
                        hitableList.push_back(make_shared<sphere>(center,0.2,make_shared<metal>(randomColor,0.5*random_double())));
                    }
                    else { //dielectric,such as glass. 透明物体
                        hitableList.push_back(make_shared<sphere>(center,0.2,make_shared<dielectric>(1.5)));
                    }
                }
            }
        }
        //Three big sphere
        hitableList.push_back( make_shared<sphere>(vec3(0,1,0), 1.0,make_shared<dielectric>(1.5)));
        hitableList.push_back( make_shared<sphere>(vec3(-4,1,0),1.0,make_shared<lambertian>(vec3(0.4,0.2,0))));
        hitableList.push_back( make_shared<sphere>(vec3(4,1,0), 1.0,make_shared<metal>(vec3(0.7,0.6,0.5),0.0)));
        return make_shared<hitable_list>(hitableList.data(), hitableList.size());
    };

    shared_ptr<hitable> world = getRandomWorldScene();

    std::vector<unsigned char> imgData;
    imgData.resize(g_Width*g_Height*3);
    if (access(imgFilePath.c_str(), 0) == 0) {
        std::remove(imgFilePath.c_str());
    }

    //Ch10: free to set aspect,and vertical-fov degree
    vec3 lookFrom(13, 2, 3);
    vec3 lookAt(0, 0, 0);
    float aperture = 0.05;
    float aspect = float(g_Width) / float(g_Height);
    //Ch11:加入焦距和光圈概念,模拟散焦模糊(景深)现象
    camera cam(20, aspect, lookFrom, lookAt, vec3(0, 1, 0), aperture,(lookFrom-lookAt).length());
    cam.set_time_interval(blurTime1, blurTime2);
    auto calcPartImg = [&](int height_start,int height_end){
        for (int j = height_start; j < height_end; ++j) {
            for (int i = 0; i < g_Width; ++i) {
                int currPixelPos = j*g_Width*3 + 3*i;
                vec3 color(0, 0, 0);
                for (int s = 0; s < g_RayNums; ++s) {
                    float u = float(i + random_double()) / float(g_Width);
                    float v = float(g_Height - 1 - j + random_double()) / float(g_Height);
                    ray r = cam.get_ray(u, v);
                    color += getColor(r, world.get(), 0);
                }
                color /= float(g_RayNums);
                int ir = int(255.99 * color.r()); imgData[currPixelPos]   = ir;
                int ig = int(255.99 * color.g()); imgData[currPixelPos+1] = ig;
                int ib = int(255.99 * color.b()); imgData[currPixelPos+2] = ib;
            }
            rtwProgress.Refresh(true);
        }
    };
    int threadNum = std::thread::hardware_concurrency();
    int averageLineNum = g_Height/threadNum;
    std::vector<std::thread> threads;
    threads.resize(threadNum);
    //多线程加速,eg:800x400的运行时间,110s加速到27s
    for(int threadIdx = 0; threadIdx < threadNum; ++threadIdx){
        int startLine = threadIdx * averageLineNum;
        int endLine =  (threadIdx == threadNum -1)? g_Height :(threadIdx+1) * averageLineNum ;
        threads[threadIdx] = std::thread(calcPartImg,startLine,endLine);
    }
    for(int threadIdx = 0; threadIdx < threadNum; ++threadIdx){
        threads[threadIdx].join();
    }

    imgFilePath.replace(imgFilePath.find(".ppm"), 4, ".bmp");
    stbi_write_bmp(imgFilePath.c_str(), g_Width, g_Height, 3, imgData.data());
    return 0;
}

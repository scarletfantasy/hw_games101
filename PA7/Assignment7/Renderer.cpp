//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include "Scene.hpp"
#include "Renderer.hpp"
#include "pthread.h"
#include <thread>
inline float deg2rad(const float& deg) { return deg * M_PI / 180.0; }

const float EPSILON = 0.00001;
struct inarg
{
    int spp;
    int bj;
    int ej;
    float scale;
    float imageAspectRatio;
    int m;
    const Scene *scene;
    std::vector<Vector3f>* framebuffer;
    Vector3f eye_pos;
};

void* pathtrace(void* args)
{
    inarg* inargs=(inarg*)args;
    auto spp=inargs->spp;
    auto bj=inargs->bj;
    auto ej=inargs->ej;
    auto scale=inargs->scale;
    auto m=inargs->m;
    auto imageAspectRatio=inargs->imageAspectRatio;
    float x = 0.0f;
    float y = 0.0f;
    for (uint32_t j = bj; j < ej; ++j) 
    {
        for (uint32_t i = 0; i < inargs->scene->width; ++i) 
        {
            // generate primary ray direction
            for (int k = 0; k < spp; k++)
            {
                float offx=get_random_float();
                float offy=get_random_float();
                x = (2 * (i + offx) / (float)inargs->scene->width - 1) *
                        imageAspectRatio * scale;
                y = (1 - 2 * (j + offy) / (float)inargs->scene->height) * scale;

                Vector3f dir = normalize(Vector3f(-x, y, 1));
                (*(inargs->framebuffer))[m] += inargs->scene->castRay(Ray(inargs->eye_pos, dir), 0) / spp;  
            }
            m++;  
        }
    }
    /* Vector3f dir = normalize(Vector3f(-x, y, 1));
    for (int k = 0; k < spp; k++){
        (*(inargs->framebuffer))[m] += inargs->scene->castRay(Ray(inargs->eye_pos, dir), 0) / spp;  
    } */
    return 0;
}
// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    std::cout<<std::thread::hardware_concurrency()<<"\n";
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = tan(deg2rad(scene.fov * 0.5));
    float imageAspectRatio = scene.width / (float)scene.height;
    Vector3f eye_pos(278, 273, -800);
    int m = 0;


    

    // change the spp value to change sample ammount
    int spp = 128;
    
    int numthreads=std::thread::hardware_concurrency();
    int groupsize=scene.height/numthreads;
    pthread_t tids[numthreads];
    inarg argarray[numthreads];
    
    std::cout << "SPP: " << spp << "\n";
    for(int i=0;i<numthreads;++i)
    {
        argarray[i].bj=i*groupsize;
        if(i==numthreads-1)
        {
            argarray[i].ej=scene.height;
        }
        else{
            argarray[i].ej=(i+1)*groupsize;
        }
        
        argarray[i].scale=scale;
        argarray[i].imageAspectRatio=imageAspectRatio;
        argarray[i].eye_pos=eye_pos;
        argarray[i].scene=&scene;
        argarray[i].framebuffer=&framebuffer;
        argarray[i].spp=spp;
        argarray[i].m=i*groupsize*scene.width;
        int ret = pthread_create(&tids[i], NULL, pathtrace, &(argarray[i]));
    }
     /* for (uint32_t j = 0; j < scene.height; ++j) {
        for (uint32_t i = 0; i < scene.width; ++i) {
            // generate primary ray direction
            float x = (2 * (i + 0.5) / (float)scene.width - 1) *
                        imageAspectRatio * scale;
            float y = (1 - 2 * (j + 0.5) / (float)scene.height) * scale;
             for (int k = 0; k < spp; k++){
                float offx=get_random_float();
                float offy=get_random_float();
                x = (2 * (i + offx) / (float)scene.width - 1) *
                        imageAspectRatio * scale;
                y = (1 - 2 * (j + offy) / (float)scene.height) * scale;

                Vector3f dir = normalize(Vector3f(-x, y, 1));
                framebuffer[m] += scene.castRay(Ray(eye_pos, dir), 0) / spp;  
            }   
            
             m++; 
        }
        UpdateProgress(j / (float)scene.height);
    } */
    UpdateProgress(1.f); 
    for(int i=0;i<numthreads;++i)
    {
        pthread_join(tids[i],NULL);
        std::cout<<i<<"\n";
    } 
    
    std::cout<<"finish\n";
    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}

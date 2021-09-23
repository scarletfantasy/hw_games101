//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}
Vector3f Scene::shade(Intersection &p,Vector3f wo) const
{
    //std::cout<<"shade\n";
    if(p.emit.norm()>EPSILON)
    {
        return p.emit;
    }
    float pdf=0.0f;
    Intersection intersection;
    sampleLight(intersection,pdf);
    auto pos=p.coords;
    Ray ray(intersection.coords,(pos-intersection.coords).normalized());
    auto dir=intersection.coords-pos;
    auto ws=dir.normalized();
    auto n=p.normal;
    auto nn=intersection.normal;
    auto sceneinter=Scene::intersect(ray);
    auto ldir=Vector3f(0.0f,0.0f,0.0f);
    //printf("%p\n",intersection.m);
    if((sceneinter.coords-pos).norm()<0.01)
    {
        ldir=intersection.emit*p.m->eval(ws,wo,n)*dotProduct(ws,n)*dotProduct(-ws,nn)/dotProduct(dir,dir)/pdf;
    }
    
    auto rr=get_random_float();
    auto lindir=Vector3f(0.0f,0.0f,0.0f);
    if(rr<RussianRoulette)
    {
        //std::cout<<"surive\n";
        auto wi=p.m->sample(wo,n).normalized();
        Ray diffuseray(pos,wi);
        auto diffuseinter=Scene::intersect(diffuseray);
        if(diffuseinter.happened)
        {
            if(diffuseinter.emit.norm()<EPSILON)
            {
                //std::cout<<"1\n";
                lindir=shade(diffuseinter,-wi)*p.m->eval(wi,wo,n)*dotProduct(wi,n)/p.m->pdf(wi,wo,n)/RussianRoulette;
            }
        }
    }
    return lindir+ldir;
    
}
// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    /* if (depth > this->maxDepth) {
        return Vector3f(0.0,0.0,0.0);
    } */
    Intersection intersection = Scene::intersect(ray);
    
    if(intersection.happened)
    {
        return shade(intersection,-ray.direction);
    }
    else
    {
        return Vector3f(0.0f,0.0f,0.0f);
    }
    /* Intersection intersection = intersect(ray);
    Vector3f hitcolor = Vector3f(0);

    //deal with light source
    if(intersection.emit.norm()>0)
    hitcolor = Vector3f(1);
    else if(intersection.happened)
    {
        Vector3f wo = normalize(-ray.direction);
        Vector3f p = intersection.coords;
        Vector3f N = normalize(intersection.normal);

        float pdf_light = 0.0f;
        Intersection inter;
        sampleLight(inter,pdf_light);
        Vector3f x = inter.coords;
        Vector3f ws = normalize(x-p);
        Vector3f NN = normalize(inter.normal);

        Vector3f L_dir = Vector3f(0);
        //direct light
        if((intersect(Ray(p,ws)).coords - x).norm() < 0.01)
        {
            L_dir = inter.emit * intersection.m->eval(wo,ws,N)*dotProduct(ws,N) * dotProduct(-ws,NN) / (((x-p).norm()* (x-p).norm()) * pdf_light);
        }

        Vector3f L_indir = Vector3f(0);
        float P_RR = get_random_float();
        //indirect light
        if(P_RR < Scene::RussianRoulette)
        {
            Vector3f wi = intersection.m->sample(wo,N);
            L_indir = castRay(Ray(p,wi),depth) *intersection.m->eval(wi,wo,N) * dotProduct(wi,N) / (intersection.m->pdf(wi,wo,N)*Scene::RussianRoulette);
        }
        hitcolor = L_indir + L_dir;
    }
    return hitcolor; */
    
    
    
}

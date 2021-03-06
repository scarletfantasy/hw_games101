#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        std::sort(objects.begin(), objects.end(), [&dim](auto f1, auto f2) {
                auto f1v=f1->getBounds().Centroid();
                auto f2v=f2->getBounds().Centroid();
                return (float)*(((float*)&(f1v))+dim) <
                       (float)*(((float*)&(f2v))+dim);
            });
        auto pbegin=(float)*(((float*)&(centroidBounds.pMin))+dim);
        auto pend=(float)*(((float*)&(centroidBounds.pMax))+dim);
        double curmin=std::numeric_limits<double>::max();
        double p=pbegin;
        int finalcount=0;
        for(auto i=pbegin;i<=pend;i+=(pend-pbegin)/8.0f)
        {
            double left=0.0f;
            double right=0.0f;
            int count=0;
            for(auto iter:objects)
            {
                auto curcenter=iter->getBounds().Centroid();
                auto mydimp=(float)*(((float*)&(curcenter))+dim);
                if(mydimp<=i)
                {
                    left+=iter->getBounds().SurfaceArea();
                    count++;
                }
                else{
                    right+=iter->getBounds().SurfaceArea();
                }
            }
            auto curres=left*count+right*(objects.size()-count);
            if(curres<curmin)
            {
                curmin=curres;
                finalcount=count;
                p=i;
            }
        }

        /* switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }
 */
        auto beginning = objects.begin();
        //auto middling = objects.begin() + (objects.size() / 2);
        auto middling = objects.begin() + finalcount;
        auto ending = objects.end();


        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    //std::cout<<isect.happened<<"\n";
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO Traverse the BVH to find intersection
    std::array<int, 3> dirisNeg = {int(ray.direction.x <0),int(ray.direction.y<0),int(ray.direction.z<0)};
    auto inbound=node->bounds.IntersectP(ray,ray.direction_inv,dirisNeg);
    Intersection intersect;
    if(!inbound)
        return intersect;
    if((node->left==NULL)&&(node->right==NULL))
    {
        return node->object->getIntersection(ray);
    }
    auto lint=getIntersection(node->left,ray);
    auto rint=getIntersection(node->right,ray);
    if(lint.distance<rint.distance)
    {
        return lint;
    }    
    else
    {
        return rint;
    }
    
    
}
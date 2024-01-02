#include <iostream>
#include <vector>
#include <cmath>

#include "VectorND.h"
#include "phystructs.h"
#include "ParticleList.h"
#include "PGrid.h"
#include "ImageUtil.h"

using namespace std;


float updateOnce(PGrid<float,2> &s,float radius,float deltasize){
    int naccept=0;
    int nproposed=0;

    //for each cell
    //
    float radius2=radius*radius;

    for(Particle<float,2> *p1 : s.updateLoop()){
        nproposed++;

        VectorND<float,2> posprop=p1->pos+VectorND<float,2>({
                ((rand()*2.0f)/RAND_MAX-1.0f)*deltasize,
                ((rand()*2.0f)/RAND_MAX-1.0f)*deltasize});

        if(posprop[0]<0||posprop[0]>s.domainMax[0]||posprop[1]<0||posprop[1]>s.domainMax[1]){
            continue; //reject proposed move
        }

        bool accept=true;

        int aind=s.getParticleIndex(*p1);
        VectorND<int,2> avec=s.indexToIntvector(aind);
        //loop over all adjacent cells
        for(int dx=-1;dx<=1 && accept;dx++){
            for(int dy=-1;dy<=1 && accept;dy++){
                if(avec[0]+dx<0
                        ||avec[0]+dx>=s.numCells[0]
                        ||avec[1]+dy<0
                        ||avec[1]+dy>=s.numCells[1])
                    continue;

                //loop over the particles in the adjacent cells
                int bind=s.intvectorToIndex(s.indexToIntvector(aind)+VectorND<int,2>({dx,dy}));
                for(size_t p2ind=0;p2ind<s.idarr[bind].size();p2ind++){
                    Particle<float,2> *p2=&((*s.plist)[s.idarr[bind][p2ind]]);
                    if(p1==p2)
                        continue;
                    if((posprop-p2->pos).length2()<4*radius2) {
                        accept=false;
                        break;
                    }
                }
            }
        }
        if(accept){
            p1->pos=posprop;
            naccept++;
        }
    } 
    return float(naccept)/nproposed;
    /*
    for(size_t aind=0;aind<s.idarr.size();aind++){
        if(s.idarr[aind].size()==0)
            continue;
        VectorND<int,2> avec=s.indexToIntvector(aind);
        
        //for each particle in the current cell
        for(size_t p1ind=0;p1ind<s.idarr[aind].size();p1ind++){
            Particle<float,2> *p1=&((*s.plist)[s.idarr[aind][p1ind]]);


            VectorND<float,2> posprop=p1->pos+VectorND<float,2>({
                    ((rand()*2.0)/RAND_MAX-1.0)*dx,
                    ((rand()*2.0)/RAND_MAX-1.0)*dx});
            bool accept=true;

            //loop over all adjacent cells
            for(int dx=-1;dx<=1 && accept;dx++){
                for(int dy=-1;dy<=1 && accept;dy++){
                    if(avec[0]+dx<0
                            ||avec[0]+dx>=s.numCells[0]
                            ||avec[1]+dy<0
                            ||avec[1]+dy>=s.numCells[1])
                        continue;

                    //loop over the particles in the adjacent cells
                    int bind=s.intvectorToIndex(s.indexToIntvector(aind)+VectorND<int,2>({dx,dy}));
                    for(size_t p2ind=0;p2ind<s.idarr[bind].size();p2ind++){
                        Particle<float,2> *p2=&((*s.plist)[s.idarr[bind][p2ind]]);
                        if(p1==p2)
                            continue;


                        if((posprop-p2->pos).length2<4*radius2)
                            accept=false;

                    }
                }
            }
        }
    } */
    //cout<<"npairs: "<<npairs<<endl;
}

/* For a hexagonal closest packing, the distance to the nearest neighbor is 2*r,
 * and the distance to the next nearest neighbor is 2*r*sqrt(3).
 * I want a function which is 1 at the nearest neighbor, and 0 at the next nearest neighbor.
 * */
float smoothWindow(float distance, float r){
    float x0=1.3f;
    float x1=sqrt(3.0f);
    x0=0.98*x1;
    float x=distance/(2.0f*r);
    if(x<x0)
        return 1.0f;
    if(x>=x1)
        return 0.0f;
    //Unique 3rd degree polynomial which satisfies:
    //{f[x0]==1,f'[x0]==0,f[x1]==0,f'[x1]==0}
    return (x-x1)*(x-x1)*(2*x-3*x0+x1)/((x1-x0)*(x1-x0)*(x1-x0));
}

int main(){

    int nparticles=10000;
    int xdim=std::floor(std::sqrt(nparticles));
    ParticleList<float,2> pl;
    float radius=0.5f/xdim;

    if(!pl.load("particletest.txt")){
        pl.initializeGrid(xdim,2*radius);
        cout<<"Creating grid from scratch."<<endl;
    } else {
        cout<<"Loading existing grid."<<endl;
    }

    
    /*for(auto p : pl.plist){
        cout<<p.pos<<endl;
    }*/
    
    PGrid<float,2> s(&pl.plist,VectorND<float,2>(1.0f),2.0*radius);

    s.rebuildGrid();

    

    /*
    float ratio=0;
    for(int i=0;i<100000;i++){
        ratio+=updateOnce(s,0.9*radius,0.1*radius);
        if(i%50000==0){
            cout<<"On step "<<i<<endl;
        }
    }
    cout<<"Acceptance ratio: "<<(ratio/1000)<<endl;
    pl.save("particletest.txt");*/

    float drawR=radius*0.9;

    std::vector<int> colors(pl.plist.size(),0);
    for(int p1ind=0;p1ind<pl.plist.size();p1ind++) {
        Particle<float,2> &p1=pl.plist[p1ind];
        float phasex=0;
        float phasey=0;

        int aind=s.getParticleIndex(p1);
        VectorND<int,2> avec=s.indexToIntvector(aind);
        //loop over all adjacent cells
        for(int dx=-1;dx<=1;dx++){
            for(int dy=-1;dy<=1;dy++){
                if(avec[0]+dx<0
                        ||avec[0]+dx>=s.numCells[0]
                        ||avec[1]+dy<0
                        ||avec[1]+dy>=s.numCells[1])
                    continue;

                //loop over the particles in the adjacent cells
                int bind=s.intvectorToIndex(s.indexToIntvector(aind)+VectorND<int,2>({dx,dy}));
                for(size_t p2ind=0;p2ind<s.idarr[bind].size();p2ind++){
                    Particle<float,2> *p2=&((*s.plist)[s.idarr[bind][p2ind]]);
                    if(&p1==p2)
                        continue;
                    VectorND<float,2> diff=p2->pos-p1.pos;
                    float hj=diff.length();
                    float phij=0;
                    if(hj>0)
                        phij=6*atan2(diff[1],diff[0]);
                    phasex+=cos(phij)*smoothWindow(hj,drawR)/6.0;
                    phasey+=sin(phij)*smoothWindow(hj,drawR)/6.0;

                }
            }
        }
        float phase=sqrt(phasex*phasex+phasey*phasey);
        if(phase>0){
            phasex/=phase;
            phasey/=phase;
            float c=(phasex+1.0f)/2.0f;
            float s=(phasey+1.0f)/2.0f;
            float m=phase>1?1:(phase<0?0:phase);
            auto rgb=hsl2rgb(0.75*c*c+0.25*s,m*0.5f+0.25f,m);
            //auto rgb=hsl2rgb(c,s,m);
            colors[p1ind]=intToRGB(rgb.r,rgb.g,rgb.b);
            if(p1ind<10){
                cout<<p1ind<<" : "<<phasex<<" "<<phasey<<" "<<phase<<endl;
                cout<<colors[p1ind]<<endl;
            }
        }
        
    } 





    int imw=3840;
    int imh=2160;

    Image outimg(imw,imh);
    float realsize=0.9;
    float cx=0.5;
    float cy=0.5;
    float aspect=float(imh)/imw;

    float drawRSquared=drawR*drawR;
    for(int a=0;a<imw;a++){
        for(int b=0;b<imh;b++){
            float x=cx+(float(a)/imw-0.5f)*realsize;
            float y=cy+(float(b)/imh-0.5f)*realsize*aspect;


            bool accept=true;
            int particleIndex=0;
            VectorND<float,2> pos({x,y});
            VectorND<int,2> avec=s.positionToIntvec(pos);
            //loop over all adjacent cells
            for(int dx=-1;dx<=1 && accept;dx++){
                for(int dy=-1;dy<=1 && accept;dy++){
                    if(avec[0]+dx<0
                            ||avec[0]+dx>=s.numCells[0]
                            ||avec[1]+dy<0
                            ||avec[1]+dy>=s.numCells[1])
                        continue;

                    //loop over the particles in the adjacent cells
                    int bind=s.intvectorToIndex(avec+VectorND<int,2>({dx,dy}));
                    for(size_t p2ind=0;p2ind<s.idarr[bind].size();p2ind++){
                        Particle<float,2> *p2=&((*s.plist)[s.idarr[bind][p2ind]]);
                        if((pos-p2->pos).length2()<drawRSquared) {
                            accept=false;
                            particleIndex=s.idarr[bind][p2ind];
                            break;
                        }
                    }
                }
            }
            if(!accept)
                outimg.put(a,b,colors[particleIndex]);
            else
                outimg.put(a,b,intToRGB(0,0,0));
        }
    }
    std::cout<<"Done, saving."<<std::endl;
    outimg.save("output1.bmp");
    /*
    int imw=3840;
    int imh=2160;
    float drawR=radius*0.9;
    

    DoubleImage dimg(imw,imh);
    float realsize=0.9;
    float cx=0.5;
    float cy=0.5;
    float aspect=float(imh)/imw;

    float drawRSquared=drawR*drawR;
    for(int a=0;a<imw;a++){
        for(int b=0;b<imh;b++){
            float x=cx+(float(a)/imw-0.5f)*realsize;
            float y=cy+(float(b)/imh-0.5f)*realsize*aspect;

            bool accept=true;
            VectorND<float,2> pos({x,y});
            VectorND<int,2> avec=s.positionToIntvec(pos);
            //loop over all adjacent cells
            for(int dx=-1;dx<=1 && accept;dx++){
                for(int dy=-1;dy<=1 && accept;dy++){
                    if(avec[0]+dx<0
                            ||avec[0]+dx>=s.numCells[0]
                            ||avec[1]+dy<0
                            ||avec[1]+dy>=s.numCells[1])
                        continue;

                    //loop over the particles in the adjacent cells
                    int bind=s.intvectorToIndex(avec+VectorND<int,2>({dx,dy}));
                    for(size_t p2ind=0;p2ind<s.idarr[bind].size();p2ind++){
                        Particle<float,2> *p2=&((*s.plist)[s.idarr[bind][p2ind]]);
                        if((pos-p2->pos).length2()<drawRSquared) {
                            accept=false;
                            break;
                        }
                    }
                }
            }
            if(accept)
                dimg.put(a,b,0.0);
            else
                dimg.put(a,b,1.0);
        }
    }
    std::cout<<"Done, saving."<<std::endl;
    //dimg.unitStretch();
    Image outimg(dimg.getData(),imw,imh);
    outimg.save("tmp.bmp");
    std::cout<<"Done."<<std::endl;*/


    /*
    int npart=0;
    auto urange=s.updateLoop();
    auto urangeend=urange.end();

    for(auto it=urange.begin();it!=urangeend;++it) {
        Particle<float,2> *particle=*it;
        npart++;
    }
    cout<<"Testing out new iterator "<<npart<<" particles."<<endl;
*/
    return 0;
}


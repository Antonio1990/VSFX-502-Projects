/*
Source: 
	http://renderman.pixar.com/resources/current/RenderMan/levelset.html#example
Pixar's webpage provides an example of how to use the implicit field plugin.
*/

#include <ImplicitField.h>
class Cube: public ImplicitField {
public:
    Cube();
    virtual ~Cube();
    virtual RtFloat Eval(const RtPoint p);
    virtual void GradientEval(RtPoint grad, const RtPoint p);
    virtual void Range(RtInterval r, const RtPoint corners[8],
        RtVolumeHandle h);
};
Cube::Cube(){
    bbox[0]=-1.;
    bbox[1]=1.;
    bbox[2]=-1.;
    bbox[3]=1.;
    bbox[4]=-1.;
    bbox[5]=1.;
}
/*
 * This is the same field falloff (as a function of the
 * square of distance from the center) that RiBlobby uses
 * for its primitive blobs.
 * It has
 *    geoff(-1)=0 geoff'(-1)=0 geoff"(-1)=0
 *      geoff( 0)=1 geoff'(0)=0
 *    geoff( 1)=0 geoff'( 1)=0 geoff"( 1)=0
 */
static float geoff(float r2){
    if(r2>=1.f) return 0.f;
    return ((3.f-r2)*r2-3.f)*r2+1.f;
}
/*
 * d geoff(r2)
 * -----------
 *    d r2
 */
static float dgeoff(float r2){
    if(r2>=1.f) return 0.f;
    return (6.f-3.f*r2)*r2-3.f;
}
/*
 * geoff(max(x^2, y^2, z^2))
 */
float Cube::Eval(const RtPoint p){
    RtPoint sq;
    float r2;

    sq[0]=p[0]*p[0];
    sq[1]=p[1]*p[1];
    sq[2]=p[2]*p[2];
    if(sq[0]>sq[1]) r2=sq[0]>sq[2]?sq[0]:sq[2];
    else r2=sq[1]>sq[2]?sq[1]:sq[2];
    return geoff(r2);
}
void Cube::GradientEval(RtPoint grad, const RtPoint p){
    RtPoint sq;

    grad[0]=0.;
    grad[1]=0.;
    grad[2]=0.;
    sq[0]=p[0]*p[0];
    sq[1]=p[1]*p[1];
    sq[2]=p[2]*p[2];
    if(sq[0]>sq[1]){
        if(sq[0]>sq[2]) grad[0]=2.*p[0]*dgeoff(sq[0]);
        else grad[2]=2.*p[2]*dgeoff(sq[2]);
    }
    else if(sq[1]>sq[2])
        grad[1]=2.*p[1]*dgeoff(sq[1]);
    else
        grad[2]=2.*p[2]*dgeoff(sq[2]);
}
void isq(RtInterval sq, RtInterval x){
    if(x[0]>=0){
        sq[0]=x[0]*x[0];
        sq[1]=x[1]*x[1];
    }
    else if(x[1]<=0){
        sq[0]=x[1]*x[1];
        sq[1]=x[0]*x[0];
    }
    else{
        sq[0]=0;
        sq[1]=-x[0]>x[1]?x[0]*x[0]:x[1]*x[1];
    }
}
void imax(RtInterval max, RtInterval a, RtInterval b){
    max[0]=b[0]>a[0]?b[0]:a[0];
    max[1]=b[1]>a[1]?b[1]:a[1];
}
void igeoff(RtInterval g, RtInterval r2){
    g[0]=geoff(r2[1]);
    g[1]=geoff(r2[0]);
}
void Cube::Range(RtInterval val, const RtPoint corners[8],
        RtVolumeHandle h){
    RtInterval x, y, z, xsq, ysq, zsq, maxxy, maxxyz;
    int i;

    x[0]=x[1]=corners[0][0];
    y[0]=y[1]=corners[0][0];
    z[0]=z[1]=corners[0][0];
    for(i=0;i!=8;i++){
        if(corners[i][0]<x[0]) x[0]=corners[i][0];
        if(corners[i][0]>x[1]) x[1]=corners[i][0];
        if(corners[i][1]<y[0]) y[0]=corners[i][1];
        if(corners[i][1]>y[1]) y[1]=corners[i][1];
        if(corners[i][2]<z[0]) z[0]=corners[i][2];
        if(corners[i][2]>z[1]) z[1]=corners[i][2];
    }
    isq(xsq, x);
    isq(ysq, y);
    isq(zsq, z);
    imax(maxxy, xsq, ysq);
    imax(maxxyz, maxxy, zsq);
    igeoff(val, maxxyz);
}
Cube::~Cube(){}
FIELDCREATE{
    return new Cube();
}

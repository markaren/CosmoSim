#ifndef COSMOSIM_H
#define COSMOSIM_H

#include "Source.h"

#if __has_include("opencv4/opencv2/opencv.hpp")
#include "opencv4/opencv2/opencv.hpp"
#else
#include "opencv2/opencv.hpp"
#endif

#define PI 3.14159265358979323846

class LensModel {
private:
    cv::Point2d eta ;  // Actual position in the source plane
    cv::Point2d nu ;   // Apparent position in the source plane

    bool centredMode = false ; // centredMode is never used
    void distort(int row, int col, const cv::Mat &src, cv::Mat &dst);
    void parallelDistort(const cv::Mat &src, cv::Mat &dst);
    cv::Mat imgDistorted;
    void updateInner();

protected:
    cv::Point2d xi ;   // Local origin in the lens plane
    cv::Point2d etaOffset = cv::Point2d(0,0) ;
        // Offset in the source plane resulting from moving xi
    double CHI;
    Source *source ;
    double einsteinR;
    int nterms;
    bool rotatedMode = true ;
    double phi{};

    int bgcolour = 0;

    double apparentAbs2 = 0;
    bool maskMode = false ;
    virtual double getMaskRadius() const ;
    void setNu( cv::Point2d ) ;

    // tentativeCentre is used as the shift when attempting 
    // to centre the distorted image in the image.
    cv::Point2d tentativeCentre = cv::Point2d(0,0) ;

    virtual void setXi( cv::Point2d ) ;
    virtual void updateApparentAbs() = 0 ;
    virtual void calculateAlphaBeta() ;
    virtual cv::Point2d getDistortedPos(double r, double theta) const = 0 ;

public:
    LensModel();
    LensModel(bool);
    ~LensModel();
    void update();
    void updateSecondary();
    void update( cv::Point2d );
    void setCentred( bool ) ;
    void setMaskMode( bool ) ;
    void setBGColour( int ) ;

    double getNuAbs() const ;
    cv::Point2d getNu() const ;
    double getXiAbs() const ;
    cv::Point2d getXi() const ;
    double getEtaAbs() const ;
    double getEtaSquare() const ;
    cv::Point2d getEta() const ;
    cv::Point2d getCentre() const ;

    void setXY(double, double, double, double) ;
    void setPolar(double, double, double, double) ;
    void setCHI(double) ;
    void setEinsteinR(double) ;
    void setNterms(int);
    void setSource(Source*) ;

    virtual void maskImage( ) ;
    virtual void markMask( ) ;
    virtual void maskImage( cv::InputOutputArray ) ;
    virtual void markMask( cv::InputOutputArray ) ;

    cv::Mat getActual() const ;
    cv::Mat getApparent() const ;
    cv::Mat getSource() const ;
    cv::Mat getDistorted() const ;
};

class PointMassLens : public LensModel { 
public:
    using LensModel::LensModel ;
protected:
    virtual cv::Point2d getDistortedPos(double r, double theta) const;
    virtual void updateApparentAbs() ;
};

/* simaux */
void refLines(cv::Mat&) ;

class NotImplemented : public std::logic_error
{
public:
    NotImplemented() : std::logic_error("Function not yet implemented") { };
};
class NotSupported : public std::logic_error
{
public:
    NotSupported() : std::logic_error("Operation not supported in this context.") { };
};

#endif // COSMOSIM_H

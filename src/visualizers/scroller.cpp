#include "scroller.h"
#include "qapplication.h"
#include "qdebug.h"
#include <math.h>
#include "qmessagebox.h"
#include "qpainterpath.h"
#include "qregularexpression.h"
#include "soundmanager.h"
Scroller::Scroller(QWidget* parent)
{

    this->parent = parent;
    m_printerText = "A VERY NICE TUNE";
    m_scrollText = "BUSINESS CLASS IS BECOMING INCREASINGLY MORE LUXURIOUS, SPACIOUS AND PRIVATE. WHETHER IT'S CUSTOM-DESIGNED SEAT AND BED CUSHIONS, BESPOKE FITTINGS AND FIXTURES, OR CO-BRANDING WITH SOME OF THE BIGGEST NAMES IN LUXURY, BUSINESS REALLY IS THE NEW FIRST CLASS ABOARD MANY PLANES.";
    customScrolltext="";

    m_vumeterWidth=100;
    m_bitmapFont = QApplication::applicationDirPath() + QString::fromUtf8("/data/resources/visualizer/bitmapfonts/anomaly.png");
    m_bitmapFontPrinter = QApplication::applicationDirPath() + QString::fromUtf8("/data/resources/visualizer/bitmapfonts/anomaly.png");
    fontScaleXPrinter=1;
    fontScaleYPrinter=1;
    m_Amplitude=32;
    sinusFrequency=0.0028f;
    sinusSpeed=0.1f;
    m_scrollSpeed=7;
    sinusFontScalingEnabled=false;
    customScrolltextEnabled=false;

    originalWidth=320;
    originalHeight=256;
    keepAspectRatio=false;
    fontScaleX=1;
    fontScaleY=2;

    starsDirection="right";
    starsEnabled=true;
    reflectionEnabled=true;
    reflectionColor=QColor(0,3,46);



    reflectionOpacity=0.04f;

    fadeOpacityPrinterText = 0;
    fadeDirectionPrinterText = 1;
    fadeWait=100;

    rasterBarsEnabled=true;
    rasterbarsOpacity=1;
    sineAngleRasterBar=0;
    counterRasterBar=0;
    numberOfRasterBars=4;
    rasterBarsVerticalSpacing=2;
    setRasterBarsBarHeight(64);
    rasterBarSpeed=50;

    hasInited=false;
    reset();

    for (int a=0; a<64; a++)
    {
        peaks[a].currentY=0;
        peaks[a].timeLeftStill=0;
        peaks[a].currentSpeedDrop=0;
    }


    previousHeight = 0;
    previousWidth = 0;
    scaleX=1;
    scaleY=1;


    initialize3dCube();


}
#define CUBE_SIZE 40
void Scroller::initialize3dCube()
{
    focalLength = 140;
        rotationSpeed = new Point3D(0.05,0.03,0.04);
        cubeAxisRotations = new Point3D(0,0,0);

        pointsArray.push_back(Point3D(-CUBE_SIZE,-CUBE_SIZE,CUBE_SIZE));
        pointsArray.push_back(Point3D(CUBE_SIZE,-CUBE_SIZE,CUBE_SIZE));
        pointsArray.push_back(Point3D(-CUBE_SIZE,CUBE_SIZE,CUBE_SIZE));
        pointsArray.push_back(Point3D(CUBE_SIZE,CUBE_SIZE,CUBE_SIZE));
        pointsArray.push_back(Point3D(-CUBE_SIZE,CUBE_SIZE,-CUBE_SIZE));
        pointsArray.push_back(Point3D(CUBE_SIZE,CUBE_SIZE,-CUBE_SIZE));
        pointsArray.push_back(Point3D(-CUBE_SIZE,-CUBE_SIZE,-CUBE_SIZE));
        pointsArray.push_back(Point3D(CUBE_SIZE,-CUBE_SIZE,-CUBE_SIZE));

        edgesArray = new int[24];
        edgesArray[0]  = 0;
        edgesArray[1]  = 1;
        edgesArray[2]  = 1;
        edgesArray[3]  = 2;
        edgesArray[4]  = 2;
        edgesArray[5]  = 3;
        edgesArray[6]  = 3;
        edgesArray[7]  = 0;
        edgesArray[8]  = 4;
        edgesArray[9]  = 5;
        edgesArray[10] = 5;
        edgesArray[11] = 6;
        edgesArray[12] = 6;
        edgesArray[13] = 7;
        edgesArray[14] = 7;
        edgesArray[15]  = 4;
        edgesArray[16]  = 0;
        edgesArray[17]  = 4;
        edgesArray[18]  = 1;
        edgesArray[19] = 5;
        edgesArray[20] = 2;
        edgesArray[21] = 6;
        edgesArray[22] = 3;
        edgesArray[23] = 7;

        for(int i = 0;i<6; i++)
        {
            facesArray.push_back(new int[3]);
        }
        facesArray[0][0]=0;
        facesArray[0][1]=1;
        facesArray[0][2]=3;
        facesArray[0][3]=2;

        facesArray[1][0]=2;
        facesArray[1][1]=3;
        facesArray[1][2]=5;
        facesArray[1][3]=4;

        facesArray[2][0]=4;
        facesArray[2][1]=5;
        facesArray[2][2]=7;
        facesArray[2][3]=6;

        facesArray[3][0]=6;
        facesArray[3][1]=7;
        facesArray[3][2]=1;
        facesArray[3][3]=0;

        facesArray[4][0]=1;
        facesArray[4][1]=7;
        facesArray[4][2]=5;
        facesArray[4][3]=3;

        facesArray[5][0]=6;
        facesArray[5][1]=0;
        facesArray[5][2]=2;
        facesArray[5][3]=4;
}

void Scroller::paint(QPainter *painter, QPaintEvent *event)
{



    if(keepAspectRatio)
    {

        float tempscaleX = float(event->rect().width())/float(originalWidth);
        float tempscaleY = float(event->rect().height())/float(originalHeight);

        scaleX = min(tempscaleX,tempscaleY);
        scaleY = scaleX;
        painter->scale(scaleX,scaleX);
    }
    else
    {
        scaleX=(qreal)event->rect().width()/originalWidth;
        scaleY=(qreal)event->rect().height()/originalHeight;
        painter->scale(scaleX,scaleY);
    }

    //This is probably not needed any longer, since changing
    //size will only scale the window, not longer extend drawing area
    if(previousHeight != originalHeight || previousWidth != originalWidth)
    {
        hasInited=false;
    }
    previousHeight = originalHeight;
    previousWidth = originalWidth;
    previousHeight = originalHeight;
    previousWidth = originalWidth;
    if(starsEnabled && !hasInited)
    {
        stars = new Star[numberOfStars];
        for (int a=0; a<numberOfStars; a++)
        {
            if(starsDirection=="in" || starsDirection=="out")
            {
                stars[a].x = rand() % 70-35;
                stars[a].y = rand() % 70-35;
            }
            else
            {
                stars[a].x = rand() % (originalWidth*2)-originalWidth;
                stars[a].y = rand() % (originalHeight*2)-originalHeight;
            }
            stars[a].speed=static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(starSpeed))); //between 0 and starSpeed;
            stars[a].z = (rand()%100 + 1);
            stars[a].coord1=0;
            stars[a].coord2=0;
        }

        hasInited=true;

    }

    if(starsEnabled)
    {
        paintStars(painter,event);
    }


    if(rasterBarsEnabled)
    {
        paintRasterBars(painter,event);
    }
    int scrollerYPosition = originalHeight / 2-fontHeight/2;
    if(reflectionEnabled)
    {
        painter->fillRect(0,scrollerYPosition+verticalScrollPosition+bottomY+fontHeight/2,originalWidth,originalHeight-scrollerYPosition+verticalScrollPosition+bottomY+fontHeight/2,reflectionColor);
    }
    painter->setOpacity(1);

    bool stereoEnabled = true;
    if (SoundManager::getInstance().m_Info1->plugin=="furnace" || SoundManager::getInstance().m_Info1->plugin=="Future Composer Player" || SoundManager::getInstance().m_Info1->plugin=="libopenmpt" || SoundManager::getInstance().m_Info1->plugin=="libxmp" || SoundManager::getInstance().m_Info1->plugin=="HivelyTracker" )
    {
        stereoEnabled=false;
    }



    if(vuMeterEnabled)
    {
        paintVUMeters(painter,event,stereoEnabled);
    }

    //paint3dCube(painter,event);

    if(printerEnabled)
    {
        printText(painter,event);
        painter->setTransform(QTransform().scale(scaleX, scaleY));
        painter->setOpacity(1);
        painter->fillRect(originalWidth,0,originalWidth*2,originalHeight,colorVisualizerBackground);

    }

    if(scrollerEnabled)
    {
        paintScroller(painter,event);
    }
    painter->setTransform(QTransform().scale(scaleX, scaleY));
    painter->setOpacity(1);
    painter->fillRect(0,originalHeight,originalWidth,originalHeight*2,colorVisualizerBackground);


}

void Scroller::printText(QPainter *painter, QPaintEvent *event)
{

    fadeWait--;
    if(fadeWait<=0)
    {
        fadeWait=0;
        fadeOpacityPrinterText+=0.01f*fadeDirectionPrinterText;
    }

    if(fadeOpacityPrinterText>1)
    {
        fadeOpacityPrinterText=1;
        fadeWait=200;
        fadeDirectionPrinterText=-1;
    }
    else if(fadeOpacityPrinterText<0)
    {
        fadeOpacityPrinterText=0;
        fadeWait=100;
        fadeDirectionPrinterText=1;
    }


    int verticalRowSpace = 2;
    int finalX = 0;
    //int finalY = (originalHeight / 2)-((fontHeightPrinter/2+verticalScrollPosition+bottomY+fontHeightPrinter/2)/2)-fontHeightPrinter/2;
    int finalY = (originalHeight / 2)-((m_printerTextRows.size()+1)*(fontHeightPrinter+verticalRowSpace));
    if(finalY<fontHeightPrinter)
    {
        finalY=fontHeightPrinter;
    }


    //reflection
    int scrollerYPosition = originalHeight / 2-fontHeightPrinter/2;
    int reflectionY = (scrollerYPosition+verticalScrollPosition+bottomY+fontHeightPrinter/2)+(fontHeightPrinter/2)*m_printerTextRows.size();
    reflectionY = reflectionY+fontHeightPrinter;

    int pos = 0;
    QString row;
    for ( int rowCount = 0 ; rowCount<m_printerTextRows.size() ; rowCount++ )
    {
        row  = m_printerTextRows.at(rowCount);



        finalX = (originalWidth/2)-((row.count()*fontWidthPrinter)/2);

        for (int n = 0; n < row.length(); n++)
        {
            int x=finalX+(n*fontWidthPrinter);
            int y=finalY+(rowCount*(fontHeightPrinter+verticalRowSpace));
            painter->setOpacity(fadeOpacityPrinterText);
            painter->setTransform(QTransform().scale(scaleX, scaleY));
            painter->drawPixmap(x, y,fontWidthPrinter,   fontHeightPrinter, m_CharacterMapPrinter, charsPrinter[pos] * fontWidthPrinter, 0,fontWidthPrinter,   fontHeightPrinter);
            //reflection

            if(reflectionEnabled)
            {
                painter->setOpacity(fadeOpacityPrinterText*reflectionOpacity);
                painter->setTransform(QTransform().scale(scaleX, -1*scaleY));
                painter->drawPixmap(x, (reflectionY+(m_printerTextRows.size()-rowCount*(fontHeightPrinter+verticalRowSpace)/2))*-1,fontWidthPrinter,   fontHeightPrinter/2, m_CharacterMapPrinter, charsPrinter[pos] * fontWidthPrinter, 0,fontWidthPrinter,   fontHeightPrinter);

            }
            pos++;

        }
    }
}
void Scroller::paintScroller(QPainter *painter, QPaintEvent *event)
{
    if (m_scrollText.isEmpty()) return;
    // Sine
    for (int n = 0; n < letters; n++)
    {
          // Draw each vertical column of pixel data, per character
          for(int xC = 0;xC < fontWidth;xC+=fontScaleX)
          {
              int y;
              if(!sinusFontScalingEnabled)
              {
                y = (sin(((x[n] + xC)*sinusFrequency) + (sineAngle)) * m_Amplitude)*fontScaleY;
              }
              else
              {
                y = (sin(((x[n] + xC)*sinusFrequency) + (sineAngle)) * m_Amplitude);
                y=y*fontScaleY;
              }


              //int y = -abs(sin(((x[n] + xC)*sinusFrequency) + (sineAngle)) * m_Amplitude);
              if(y>bottomY)
              {
                  bottomY=y;
              }

              //Only draw if it's visible
              int scrollerYPosition = originalHeight / 2-fontHeight/2;
              int finalY = y+scrollerYPosition+verticalScrollPosition;

              if(x[n]+(xC)>(-1*fontWidth) && x[n]+(xC)<originalWidth && finalY<originalHeight &&  finalY>(-1*fontHeight))
              {
                painter->setOpacity(1.0f);
                painter->setTransform(QTransform().scale(scaleX, scaleY));
                painter->drawPixmap(x[n]+(xC), finalY,    fontScaleX,   fontHeight, m_CharacterMap, chars[n] * fontWidth + xC, 0,      fontScaleX,   fontHeight);
                //reflection
                if(reflectionEnabled)
                {
                    painter->setOpacity(reflectionOpacity);
                    painter->setTransform(QTransform().scale(scaleX, -1*scaleY));
                    painter->drawPixmap(x[n]+(xC), y-scrollerYPosition-verticalScrollPosition-(bottomY*2)-fontHeight-(fontHeight/2),    fontScaleX,   fontHeight/2, m_CharacterMap, chars[n] * fontWidth + xC, 0,      fontScaleX,   fontHeight);
                }
              }
          }
          painter->setOpacity(1.0f);

          // Scroll speed
          //Don't scroll if paused
          if(!SoundManager::getInstance().GetPaused())
          {
            x[n]-=m_scrollSpeed;
          }


          // Next letter
          if (x[n] < -(fontWidth))
          {
              x[n] = (letters - 1) * (fontWidth);
              int charPos;
              charPos = bitmapFontCharset.indexOf(m_scrollText.at(position));
              chars[n]=charPos;
              position++;
              if (position > m_scrollText.length()) position = 0;
          }
      }

      // Sine speed
      sineAngle+=sinusSpeed;
}

void Scroller::paint3dCube(QPainter *painter, QPaintEvent *event)
{

    painter->setOpacity(1.0f);
    double amp;
    amp = SoundManager::getInstance().getSoundData(0); //volume is between 0-100

    amp = amp/30.0;


    QPainterPath myPath;

    rotationSpeed->x = 0.05*amp*2;
    rotationSpeed->y = 0.03*amp*2;
    rotationSpeed->z = 0.04*amp*2;

    cubeAxisRotations->y -= rotationSpeed->y;
    cubeAxisRotations->x -=  rotationSpeed->x;
    cubeAxisRotations->z -=  rotationSpeed->z;

    std::vector<Point2D> screenPoints;
    transform3DPointsTo2DPoints(pointsArray, cubeAxisRotations, &screenPoints);

    for(int i = 0 ; i < 6 ; i++)
    {
//        myPath.moveTo (originalWidth/2 + screenPoints.at(facesArray[i][0]).x,originalHeight/2 + screenPoints.at(facesArray[i][0]).y);
//        QPolygonF poly;
        for(int j = 0 ; j < 3 ; j++)
        {
//            myPath.lineTo (originalWidth/2 + screenPoints.at(facesArray[i][j]).x,originalHeight/2 + screenPoints.at(facesArray[i][j]).y);
//            painter->drawLine(originalWidth/2 + screenPoints.at(facesArray[i][j]).x,originalHeight/2 + screenPoints.at(facesArray[i][j]).y,originalWidth/2 + screenPoints.at(facesArray[i][j+1]).x,originalHeight/2 + screenPoints.at(facesArray[i][j+1]).y);
//            painter->drawPoint(originalWidth/2 + screenPoints.at(facesArray[i][j]).x,originalHeight/2 + screenPoints.at(facesArray[i][j]).y);
            //painter->drawLine(originalWidth/2 + screenPoints.at(facesArray[i][j]).x,originalHeight/2 + screenPoints.at(facesArray[i][j]).y,originalWidth/2 + screenPoints.at(facesArray[i][j+1]).x,originalHeight/2 + screenPoints.at(facesArray[i][j+1]).y);
        }


//        myPath.closeSubpath();


    }


    QColor c(255,0,255);

    painter->setPen(c);
    painter->setBrush(c);
    painter->drawLine(0,50,300,200);

}

void Scroller::paintRasterBars(QPainter *painter, QPaintEvent *event)
{

    counterRasterBar+=0.001*rasterBarSpeed;
    sineAngleRasterBar+=0.004;
    for (int i=0;i<numberOfRasterBars;i++)
    {
        createRasterBar(painter,i,numberOfRasterBars);
    }
}

void Scroller::createRasterBar(QPainter *painter, int offset, int numBars)
{
    painter->setOpacity(rasterbarsOpacity);
    int HEIGHT = originalHeight;
    int WIDTH = originalWidth;
    double amp = (HEIGHT/2)-(rasterBarHeight/2);
    double y = (((originalHeight/2)-numBars)+((sin(counterRasterBar + offset/double(numBars*rasterBarsVerticalSpacing*0.05) +(sineAngleRasterBar)) * amp)))+numBars-(rasterBarHeight/2);




    int colorNum;
    for(int i = 0; i< rasterBarHeight; i++)
    {
        colorNum=i;
        if(colorNum>(rasterBarHeight/2)-1)
        {
            colorNum=rasterBarHeight-1-i;
        }
        painter->setPen(rasterBarsColors.at(offset).at(colorNum));
        painter->drawLine(0,y+i,WIDTH,y+i);
    }
    painter->setOpacity(1);

}

void Scroller::paintVUMeters(QPainter *painter, QPaintEvent *event, bool stereo)
{

    int numChannels=SoundManager::getInstance().m_Info1->numChannels;
    if(stereo)
    {
        numChannels=SoundManager::getInstance().m_Info1->numChannelsStream;
    }
    int maxHeight;
    if(reflectionEnabled)
    {
        maxHeight = originalHeight / 2-fontHeight/2+verticalScrollPosition+bottomY+fontHeight/2;
    }
    else
    {
        maxHeight = originalHeight;
    }

    if(vumeterPeaksEnabled)
    {
        maxHeight -= heightPeak;
    }


    int VUMETERWIDTH = (originalWidth/numChannels)*(m_vumeterWidth/100.0);
    int WIDTH = (originalWidth/numChannels);

    int HILIGHT_WIDTH = VUMETERWIDTH*0.1;


    int LEFT_OFFSET = (WIDTH-VUMETERWIDTH)/2;

    for(int unsigned i = 0; i < numChannels ; i++)
    {

        unsigned char volume;
        if(stereo)
        {
            volume = SoundManager::getInstance().getSoundData(i); //volume is between 0-100
        }
        else
        {
            volume = static_cast<unsigned char>(SoundManager::getInstance().m_Info1->modVUMeters[i]); //volume is between 0-100
        }


        int vumeterCurrentHeight = (volume*maxHeight/100);

        int vuWidth = VUMETERWIDTH;
        int xPos = (i*WIDTH)+LEFT_OFFSET;

        QRect rectL;
        QRect rectLHiLite;
        QRect rectLDark;
        QLinearGradient linearGrad;

        if(vumeterPeaksEnabled)
        {
            rectL = QRect(xPos,maxHeight-vumeterCurrentHeight+heightPeak,vuWidth,vumeterCurrentHeight);
            rectLHiLite = QRect(xPos,maxHeight-vumeterCurrentHeight+heightPeak,HILIGHT_WIDTH,vumeterCurrentHeight);
            rectLDark = QRect(xPos+vuWidth-HILIGHT_WIDTH,maxHeight-vumeterCurrentHeight+heightPeak,HILIGHT_WIDTH,vumeterCurrentHeight);
            linearGrad = QLinearGradient(QPointF(0, heightPeak), QPointF(0, maxHeight+heightPeak));
        }
        else
        {
            rectL = QRect(xPos,maxHeight-vumeterCurrentHeight,vuWidth,vumeterCurrentHeight);
            rectLHiLite = QRect(xPos,maxHeight-vumeterCurrentHeight,HILIGHT_WIDTH,vumeterCurrentHeight);
            rectLDark = QRect(xPos+vuWidth-HILIGHT_WIDTH,maxHeight-vumeterCurrentHeight,HILIGHT_WIDTH,vumeterCurrentHeight);
            linearGrad = QLinearGradient(QPointF(0, 0), QPointF(0, maxHeight));
        }

        //main color
        linearGrad.setColorAt(0, colorVisualizerTop);
        linearGrad.setColorAt(0.5, colorVisualizerMiddle);
        linearGrad.setColorAt(1, colorVisualizerBottom);
        painter->setOpacity(vumeterOpacity);

        painter->fillRect(rectL,QBrush(linearGrad));
        painter->fillRect(rectLHiLite,QColor(255,255,255,50));
        painter->fillRect(rectLDark,QColor(0,0,0,80));

        //peaks

        if(vumeterPeaksEnabled)
        {
            if(vumeterCurrentHeight>=peaks[i].currentY)
            {
                peaks[i].currentY=vumeterCurrentHeight;
                peaks[i].timeLeftStill=10;
                peaks[i].currentSpeedDrop=1;
            }
            else
            {
                if(peaks[i].timeLeftStill<=0)
                {
                    peaks[i].currentSpeedDrop++;
                    peaks[i].currentY-=peaks[i].currentSpeedDrop;
                }
                else
                {
                    peaks[i].timeLeftStill-=1;
                }

            }

            QRect peak(xPos,maxHeight-peaks[i].currentY,vuWidth,heightPeak);
            QRect peakHiLite(xPos,maxHeight-peaks[i].currentY,HILIGHT_WIDTH,heightPeak);
            QRect peakDark(xPos+vuWidth-HILIGHT_WIDTH,maxHeight-peaks[i].currentY,HILIGHT_WIDTH,heightPeak);


            painter->fillRect(peak,colorVisualizerPeak);
            painter->fillRect(peakHiLite,QColor(255,255,255,50));
            painter->fillRect(peakDark,QColor(0,0,0,80));


        }

        if(reflectionEnabled)
        {
            painter->setOpacity(reflectionOpacity*vumeterOpacity);

            QLinearGradient linearGradReflection;



            QRect rectL;
            QRect rectLHiLite;
            QRect rectLDark;

            if(vumeterPeaksEnabled)
            {
                rectL = QRect(xPos,maxHeight+heightPeak,vuWidth,vumeterCurrentHeight/2);
                rectLHiLite = QRect(xPos,maxHeight+heightPeak,HILIGHT_WIDTH,vumeterCurrentHeight/2);
                rectLDark = QRect(xPos+vuWidth-HILIGHT_WIDTH,maxHeight+heightPeak,HILIGHT_WIDTH,vumeterCurrentHeight/2);
                linearGradReflection = QLinearGradient(QPointF(0, maxHeight+heightPeak), QPointF(0, (maxHeight*1.5)+heightPeak));

                QRect peak(xPos,maxHeight+(heightPeak+peaks[i].currentY/2),vuWidth,heightPeak/2);
                QRect peakHiLite(xPos,maxHeight+(heightPeak+peaks[i].currentY/2),HILIGHT_WIDTH,heightPeak/2);
                QRect peakDark(xPos+vuWidth-HILIGHT_WIDTH,maxHeight+(heightPeak+peaks[i].currentY/2),HILIGHT_WIDTH,heightPeak/2);

                painter->fillRect(peak,colorVisualizerPeak);
                painter->fillRect(peakHiLite,QColor(255,255,255,50));
                painter->fillRect(peakDark,QColor(0,0,0,80));
            }
            else
            {
                rectL = QRect(xPos,maxHeight,vuWidth,vumeterCurrentHeight/2);
                rectLHiLite = QRect(xPos,maxHeight,HILIGHT_WIDTH,vumeterCurrentHeight/2);
                rectLDark = QRect(xPos+vuWidth-HILIGHT_WIDTH,maxHeight,HILIGHT_WIDTH,vumeterCurrentHeight/2);
                linearGradReflection = QLinearGradient(QPointF(0, maxHeight), QPointF(0, maxHeight*1.5));
            }


            //main color
            linearGradReflection.setColorAt(0, colorVisualizerBottom);
            linearGradReflection.setColorAt(0.5, colorVisualizerMiddle);
            linearGradReflection.setColorAt(1, colorVisualizerTop);




            painter->fillRect(rectL,QBrush(linearGradReflection));
            painter->fillRect(rectLHiLite,QColor(255,255,255,50));
            painter->fillRect(rectLDark,QColor(0,0,0,80));




        }

    }
    painter->setOpacity(1.0);
}
void Scroller::paintStars(QPainter *painter, QPaintEvent *event)
{
    int HEIGHT = originalHeight;
    int WIDTH = originalWidth;


    for (int i=0; i<numberOfStars; i++)
    {

        if(starsDirection=="right")
        {
            stars[i].x+=stars[i].speed;
        }
        else if(starsDirection=="left")
        {
            stars[i].x-=stars[i].speed;
        }
        else if(starsDirection=="up")
        {
            stars[i].y-=stars[i].speed;
        }
        else if(starsDirection=="down")
        {
            stars[i].y+=stars[i].speed;
        }
        else if(starsDirection=="in" || starsDirection=="out")
        {
            if(starsDirection=="in")
            {
                stars[i].z -= starSpeed*0.2;
                if (stars[i].z <= 0)
                {
                    // re-initialize if star is too close
                    stars[i].x = (float)(rand()%70)-35;
                    stars[i].y = (float)(rand()%70)-35;
                    stars[i].speed=static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(starSpeed))); //between 0 and starSpeed;
                    stars[i].z +=100;
                }
            }
            else if(starsDirection=="out")
            {
                stars[i].z += starSpeed*0.2;
                if (stars[i].z >= 100)
                {
                    // re-initialize if star is too far away
                    stars[i].x = (float)(rand()%70)-35;
                    stars[i].y = (float)(rand()%70)-35;
                    stars[i].speed=static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(starSpeed))); //between 0 and starSpeed;
                    stars[i].z -=100;
                }
            }

            stars[i].coord1 = (stars[i].x / stars[i].z) * WIDTH + (WIDTH>>1);
            stars[i].coord2 = (stars[i].y / stars[i].z) * HEIGHT + (HEIGHT>>1);

            if(stars[i].coord1>=0 && stars[i].coord1<=originalWidth && stars[i].coord2>=0 && stars[i].coord2<=originalHeight)
            {
                float percentMaxspeed = (stars[i].z/ 100);
                QColor c(255,255,255,255-(255*percentMaxspeed));

                painter->setPen(c);
                painter->setBrush(c);
                //painter->drawEllipse(QPointF(stars[i].x,stars[i].y), 1, 1);

                int x = stars[i].coord1;
                int y = stars[i].coord2;
                int z = stars[i].z;

                painter->drawPoint(QPointF(x,y));
            }
        }

        if(starsDirection!="in" && starsDirection!="out" && stars[i].x>=0 && stars[i].x<=originalWidth && stars[i].y>=0 && stars[i].y<=originalHeight)
        {
            float percentMaxspeed = stars[i].speed / starSpeed;
            QColor c(255,255,255,percentMaxspeed*255);

            painter->setPen(c);
            painter->setBrush(c);
            //painter->drawEllipse(QPointF(stars[i].x,stars[i].y), 1, 1);
            painter->drawPoint(QPointF(stars[i].x,stars[i].y));
        }


        if(starsDirection=="right")
        {
            if(stars[i].x>originalWidth)
            {
                stars[i].x = (rand() % WIDTH)-WIDTH;
                stars[i].y = rand() % HEIGHT;
                stars[i].speed=static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(starSpeed))); //between 0 and starSpeed;
            }
        }
        else if(starsDirection=="left")
        {
            if(stars[i].x<0)
            {
                stars[i].x = (rand() % WIDTH)+WIDTH;
                stars[i].y = rand() % HEIGHT;
                stars[i].speed=static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(starSpeed))); //between 0 and starSpeed;
            }
        }
        else if(starsDirection=="up")
        {
            if(stars[i].y<0)
            {
                stars[i].x = rand() % WIDTH;
                stars[i].y = (rand() % HEIGHT) + HEIGHT ;
                stars[i].speed=static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(starSpeed))); //between 0 and starSpeed;
            }
        }
        else if(starsDirection=="down")
        {
            if(stars[i].y>originalHeight)
            {
                stars[i].x = rand() % WIDTH;
                stars[i].y = (rand() % HEIGHT) - HEIGHT ;
                stars[i].speed=static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(starSpeed))); //between 0 and starSpeed;
            }
        }
    }
}
void Scroller::setColorVisualizerTop(QColor newColor)
{
    colorVisualizerTop=newColor;
}
void Scroller::setColorVisualizerBottom(QColor newColor)
{
    colorVisualizerBottom=newColor;
}
void Scroller::setColorVisualizerMiddle(QColor newColor)
{
    colorVisualizerMiddle=newColor;
}
void Scroller::setColorVisualizerBackground(QColor newColor)
{
    colorVisualizerBackground=newColor;
}
void Scroller::setHeightVisualizerPeak(int height)
{
    heightPeak=height;
}
void Scroller::setColorVisualizerPeakColor(QColor newColor)
{
    colorVisualizerPeak=newColor;
}
void Scroller::setVUMeterPeaksEnabled(bool enable)
{
    vumeterPeaksEnabled=enable;
}
void Scroller::setVumeterWidth(double width)
{
    m_vumeterWidth=width;
}
void Scroller::setReflectionOpacity(int opacity)
{
    reflectionOpacity=opacity/100.0;
}
QColor Scroller::getColorVisualizerBackground()
{
    return colorVisualizerBackground;
}
int Scroller::getReflectionOpacity()
{
    return reflectionOpacity*100;
}
bool Scroller::getSinusFontScalingEnabled()
{
    return sinusFontScalingEnabled;
}
qreal Scroller::getScaleX()
{
    return scaleX;
}
qreal Scroller::getScaleY()
{
    return scaleY;
}
int Scroller::getResolutionWidth()
{
    return originalWidth;
}
int Scroller::getResolutionHeight()
{
    return originalHeight;
}
bool Scroller::getKeepAspectRatio()
{
    return keepAspectRatio;
}
void Scroller::setVumeterOpacity(int opacity)
{
    vumeterOpacity=opacity/100.0;
}
int Scroller::getVumeterOpacity()
{
    return vumeterOpacity*100;
}
int Scroller::getRasterbarsOpacity()
{
    return rasterbarsOpacity*100;
}
void Scroller::stop()
{
    isStopping=true;
}
bool Scroller::getStarsEnabled()
{
    return starsEnabled;
}
int Scroller::getNumberOfStars()
{
    return numberOfStars;
}
int Scroller::getStarSpeed()
{
    return starSpeed;
}
bool Scroller::getVUMeterEnabled()
{
    return vuMeterEnabled;
}
bool Scroller::getScrollerEnabled()
{
    return scrollerEnabled;
}
bool Scroller::getCustomScrolltextEnabled()
{
    return customScrolltextEnabled;
}
QString Scroller::getCustomScrolltext()
{
    return customScrolltext;
}
bool Scroller::getPrinterEnabled()
{
    return printerEnabled;
}
int Scroller::getAmplitude()
{
    return m_Amplitude;
}

double Scroller::getVumeterWidth()
{
    return m_vumeterWidth;
}
double Scroller::getFrequency()
{
    return sinusFrequency;
}
double Scroller::getSinusSpeed()
{
    return sinusSpeed;
}
int Scroller::getScrollSpeed()
{
    return m_scrollSpeed;
}
int Scroller::getVerticalScrollPosition()
{
    return verticalScrollPosition;
}
int Scroller::getFontScaleX()
{
    return fontScaleX;
}
int Scroller::getFontScaleY()
{
    return fontScaleY;
}
int Scroller::getPrinterFontScaleX()
{
    return fontScaleXPrinter;
}
int Scroller::getPrinterFontScaleY()
{
    return fontScaleYPrinter;
}
bool Scroller::getReflectionEnabled()
{
    return reflectionEnabled;
}
QString Scroller::getReflectionColor()
{
    return reflectionColor.name();
}
QString Scroller::getFont()
{
    return m_bitmapFont;
}
QString Scroller::getPrinterFont()
{
    return m_bitmapFontPrinter;
}
bool Scroller::getRasterBarsEnabled()
{
    return rasterBarsEnabled;
}
int Scroller::getNumberOfRasterBars()
{
    return numberOfRasterBars;
}
int Scroller::getRasterBarsSpeed()
{
    return rasterBarSpeed;
}
int Scroller::getRasterBarsVerticalSpacing()
{
    return 33-rasterBarsVerticalSpacing;
}
int Scroller::getRasterBarsHeight()
{
    return rasterBarHeight;
}
void Scroller::setScrollText(QString text)
{
    int pixelsPerCharacter = fontWidth;
    int spacesNeeded = (originalWidth/pixelsPerCharacter)+1;
    QString spaces=" ";
    spaces = spaces.repeated(spacesNeeded);


    if(QString(text.toUpper()).trimmed().isEmpty())
    {
        m_scrollText="";
    }
    else
    {
        m_scrollText=spaces+QString(text.toUpper()).trimmed();
    }
    m_scrollText = replaceIllegalLetters(m_scrollText);
    letters=spacesNeeded+1; //Added one because some fontsizes will popup into the screen otherwise
}
QString Scroller::replaceIllegalLetters(QString text)
{
    QString a1 = "ÀÁÂÃÄÅĀĂáàāãåä";
    QString a2 = "A";
    QString b1 = "ß";
    QString b2 = "B";
    QString c1 = "Çç";
    QString c2 = "C";
    QString e1 = "ËÉÈÊèéêë";
    QString e2 = "E";
    QString f1 = "ƒ";
    QString f2 = "F";
    QString i1 = "ÍÌÎÏìíîï";
    QString i2 = "I";
    QString n1 = "Ññ";
    QString n2 = "N";
    QString o1 = "ØÓÒÔÕÖòóôðøõöø";
    QString o2 = "O";
    QString u1 = "ÛÙÚÜùúüûµ";
    QString u2 = "U";
    QString x1 = "×";
    QString x2 = "X";
    QString y1 = "Ýýÿ";
    QString y2 = "Y";
    QString question1 = "¿";
    QString question2 = "?";
    QString exclamation1 = "¡";
    QString exclamation2     = "!";
    QString hyphen1 = "¯─~·";
    QString hyphen2     = "-";
    QString eq1 = "═‗≈";
    QString eq2     = "=";
    QString quote1 = "`´‘’“”";
    QString quote2     = "\"";
    QString one1 = "¹";
    QString one2     = "1";
    QString two1 = "²";
    QString two2     = "2";
    QString three1 = "³";
    QString three2     = "3";
    QString half1 = "½";
    QString half2 = "1/2";
    QString quarter1 = "¼";
    QString quarter2 = "1/4";
    QString threequarter1 = "¾";
    QString threequarter2 = "3/4";
    QString arrow1 = "→";
    QString arrow2     = "->";


    // Performance: Check for characters
    if (text.contains(QRegularExpression("[" + QRegularExpression::escape("$/:" + a1 + b1 + c1 + e1 + f1 + i1 + n1 + o1 + u1 + x1 + y1 + question1 + exclamation1 + hyphen1 + eq1 + quote1 + one1 + two1 + three1 + arrow1 + half1 + quarter1 + threequarter1) + "]")))
    {
        text.replace(QRegularExpression("[ëêéè]"), "e");
    }
    text.replace(QRegularExpression("["+a1+"]"), a2);
    text.replace(QRegularExpression("["+b1+"]"), b2);
    text.replace(QRegularExpression("["+c1+"]"), c2);
    text.replace(QRegularExpression("["+e1+"]"), e2);
    text.replace(QRegularExpression("["+f1+"]"), f2);
    text.replace(QRegularExpression("["+i1+"]"), i2);
    text.replace(QRegularExpression("["+n1+"]"), n2);
    text.replace(QRegularExpression("["+o1+"]"), o2);
    text.replace(QRegularExpression("["+u1+"]"), u2);
    text.replace(QRegularExpression("["+x1+"]"), x2);
    text.replace(QRegularExpression("["+y1+"]"), y2);
    text.replace(QRegularExpression("["+question1+"]"), question2);
    text.replace(QRegularExpression("["+exclamation1+"]"), exclamation2);
    text.replace(QRegularExpression("["+hyphen1+"]"), hyphen2);
    text.replace(QRegularExpression("["+eq1+"]"), eq2);
    text.replace(QRegularExpression("["+quote1+"]"), quote2);
    text.replace(QRegularExpression("["+one1+"]"), one2);
    text.replace(QRegularExpression("["+two1+"]"), two2);
    text.replace(QRegularExpression("["+three1+"]"), three2);
    text.replace(QRegularExpression("["+half1+"]"), half2);
    text.replace(QRegularExpression("["+quarter1+"]"), quarter2);
    text.replace(QRegularExpression("["+threequarter1+"]"), threequarter2);
    text.replace(QRegularExpression("["+arrow1+"]"), arrow2);

    return text;
}

void Scroller::setPrinterText(QString text)
{
    reset();
    m_printerTextRows.clear();
    m_printerText=QString(text.toUpper()).trimmed();
    int maxNumberOfCharactersPerRow = originalWidth/fontWidthPrinter;
    //first split to rows for every space character
    QString row="";

    for(int i = 0 ; i < m_printerText.length(); i++)
    {
        row+=m_printerText.at(i);
        if(m_printerText.at(i)==' ' || m_printerText.at(i)=='-' || m_printerText.at(i)=='.' || row.length()==maxNumberOfCharactersPerRow)
        {
            if(row.trimmed()!="")
            {
                m_printerTextRows.append(row);
            }
            row="";
        }
    }
    m_printerTextRows.append(row.trimmed());
    QStringList newList;
    //merge rows that fit in one row
    //TODO will only merge max two short rows, should be ininite
    for ( int i = 0 ; i < m_printerTextRows.count() ; i++ )
    {
        if(i<m_printerTextRows.count()-1)
        {
            if(m_printerTextRows.at(i).length() + m_printerTextRows.at(i+1).length()<=maxNumberOfCharactersPerRow)
            {
               newList.append(QString(m_printerTextRows.at(i) + m_printerTextRows.at(i+1)).trimmed());
               i++;
            }
            else
            {
                newList.append(m_printerTextRows.at(i).trimmed());
            }
        }
        else
        {
            newList.append(m_printerTextRows.at(i).trimmed());
        }
    }
    m_printerTextRows=newList;

    reset();
}
void Scroller::setSinusSpeed(double v){
    sinusSpeed=v;
    bottomY=0;
}
void Scroller::setSinusFrequency(double v){
    sinusFrequency=v;
    bottomY=0;
}

void Scroller::setAmplitude(int v){
    m_Amplitude=v;
    bottomY=0;
}
void Scroller::setScrollSpeed(int v)
{
    m_scrollSpeed=v;
    bottomY=0;
}
void Scroller::reset()
{
    setScrollerFont(m_bitmapFont);
    setPrinterFont(m_bitmapFontPrinter);
    fadeOpacityPrinterText = 0;
    fadeDirectionPrinterText = 1;
    fadeWait=100;

}
void Scroller::setFontScaleX(int v)
{
    fontScaleX=v;
    setPrinterText(m_printerText);
    reset();


}
void Scroller::setFontScaleY(int v)
{
    fontScaleY=v;
    setPrinterText(m_printerText);
    reset();

}
void Scroller::setPrinterFontScaleX(int v)
{
    fontScaleXPrinter=v;
    reset();


}
void Scroller::setPrinterFontScaleY(int v)
{
    fontScaleYPrinter=v;
    reset();

}
bool Scroller::setScrollerFont(QString font)
{
    if (m_scrollText.isEmpty()) return false;

    int extensionPos = font.lastIndexOf('.');
    if (extensionPos==-1)
    {
        return false;
    }
    QFile myFile(font.left(extensionPos)+".inf");
    if (myFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream( &myFile );
        QString line = stream.readLine();
        fontWidthOriginal=line.toInt();
        line = stream.readLine();
        fontHeightOriginal=line.toInt();
        line = stream.readLine();
        bitmapFontCharset=line;
        m_bitmapFont = font;
        m_CharacterMap = QPixmap(m_bitmapFont);

        m_CharacterMap = m_CharacterMap.scaled(m_CharacterMap.width()*fontScaleX,m_CharacterMap.height()*fontScaleY);
        fontWidth=fontWidthOriginal*fontScaleX;
        fontHeight=fontHeightOriginal*fontScaleY;
        position=letters;
        sineAngle = 0;
        bottomY=0;
        setScrollText(m_scrollText);

        for (int n = 0; n < letters; n++)
        {
          int charPos;
          charPos = bitmapFontCharset.indexOf(m_scrollText.at(n));
          chars[n]=charPos;
          x[n] = n * fontWidth;
        }

        return true;
    }
    return false;
}
bool Scroller::setPrinterFont(QString font)
{

    int extensionPos = font.lastIndexOf('.');
    if (extensionPos==-1)
    {
        return false;
    }
    QFile myFile(font.left(extensionPos)+".inf");
    if (myFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream( &myFile );
        QString line = stream.readLine();
        fontWidthOriginalPrinter=line.toInt();
        line = stream.readLine();
        fontHeightOriginalPrinter=line.toInt();
        line = stream.readLine();
        bitmapFontCharsetPrinter=line;
        m_bitmapFontPrinter = font;
        m_CharacterMapPrinter = QPixmap(m_bitmapFontPrinter);

        m_CharacterMapPrinter = m_CharacterMapPrinter.scaled(m_CharacterMapPrinter.width()*fontScaleXPrinter,m_CharacterMapPrinter.height()*fontScaleYPrinter);
        fontWidthPrinter=fontWidthOriginalPrinter*fontScaleXPrinter;
        fontHeightPrinter=fontHeightOriginalPrinter*fontScaleYPrinter;

        QString row;
        int pos = 0;
        for (int rowCount = 0 ; rowCount<m_printerTextRows.size() ; rowCount++)
        {
          row  = m_printerTextRows.at(rowCount);
          for (int n = 0; n < row.length(); n++)
          {
            int charPos;
            charPos = bitmapFontCharsetPrinter.indexOf(row.at(n));
            charsPrinter[pos] = charPos;
            pos++;
          }
        }
        return true;
    }
    return false;
}

void Scroller::setReflectionEnabled(bool enabled)
{
    reflectionEnabled=enabled;
}
void Scroller::setScrollerReflectionColor(QColor newColor)
{
    reflectionColor = newColor;
}
void Scroller::setSinusFontScalingEnabled(bool enabled)
{
    sinusFontScalingEnabled = enabled;
}
void Scroller::setKeepAspectRatio(bool enabled)
{
    keepAspectRatio=enabled;
}
void Scroller::setResolutionWidth(int width)
{
    originalWidth = width;
    reset();
}
void Scroller::setResolutionHeight(int height)
{
    originalHeight = height;
}
void Scroller::setStarsEnabled(bool enabled)
{
    starsEnabled=enabled;
}
void Scroller::setStarsDirection(QString direction)
{
    starsDirection=direction;
}
QString Scroller::getStarsDirection()
{
    return starsDirection;
}
void Scroller::setVUMeterEnabled(bool enabled)
{
    vuMeterEnabled=enabled;
}
void Scroller::setPrinterEnabled(bool enabled)
{
    printerEnabled=enabled;
}
void Scroller::setCustomScrolltextEnabled(bool enabled)
{
    customScrolltextEnabled=enabled;
}
void Scroller::setCustomScrolltext(QString text)
{
    customScrolltext=text;
}
void Scroller::setRasterBarsEnabled(bool enabled)
{
    rasterBarsEnabled=enabled;
}
void Scroller::setNumberOfStars(int amount)
{
    numberOfStars=amount;
    if(hasInited)
    {
        delete[] stars;
    }
    hasInited=false;
}
void Scroller::setRasterBarsVerticalSpacing(int amount)
{
    rasterBarsVerticalSpacing=33-amount;
}
void Scroller::setRasterBarsBarHeight(int amount)
{
    rasterBarHeight=amount;
    rasterBarsColors.clear();


    for(int n = 0; n<numberOfRasterBars ; n++)
    {
        double randHue = ((double) rand() / (RAND_MAX));
        double randSaturation = ((double) rand() / (RAND_MAX));
        QList<QColor> colors;
        for(int i = 0; i<= (rasterBarHeight/2) ; i++)
        {
            QColor color;
            color.setHsvF(randHue, randSaturation, (double)i/(rasterBarHeight/2));
            colors.append(color);
        }
        rasterBarsColors.append(colors);
    }
}
void Scroller::setRasterBarsSpeed(int amount)
{
    rasterBarSpeed=amount;
}
void Scroller::setRasterBarsOpacity(int amount)
{
    rasterbarsOpacity=amount/100.0;
}
void Scroller::setNumberOfRasterBars(int amount)
{
    numberOfRasterBars=amount;
    setRasterBarsBarHeight(rasterBarHeight); //just to create new colors
}
void Scroller::setStarSpeed(int speed)
{
    starSpeed=speed;
    if(starsEnabled && hasInited)
    {
        for (int a=0; a<numberOfStars; a++)
        {
            stars[a].speed=static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(starSpeed))); //between 0 and starSpeed;
        }
    }
}
void Scroller::setScrollerEnabled(bool enabled)
{
    scrollerEnabled=enabled;
}
void Scroller::setVerticalScrollPosition(int v)
{
    verticalScrollPosition=v;
    bottomY=0;
}

void Scroller::transform3DPointsTo2DPoints (std::vector<Point3D> points, Point3D* axisRotations, std::vector<Point2D>* TransformedPointsArray)
{
    // the array to hold transformed 2D points - the 3D points
    // from the point array which are here rotated and scaled
    // to generate a point as it would appear on the screen
    //TransformedPointsArray* = new Point2D[8];
    // Math calcs for angles - sin and cos for each (trig)
    // this will be the only time sin or cos is used for the
    // entire portion of calculating all rotations
    double sx = sin(axisRotations->x);
    double cx = cos(axisRotations->x);
    double sy = sin(axisRotations->y);
    double cy = cos(axisRotations->y);
    double sz = sin(axisRotations->z);
    double cz = cos(axisRotations->z);

    // a couple of variables to be used in the looping
    // of all the points in the transform process
    double x,y,z, xy,xz, yx,yz, zx,zy, scaleRatio;

    // 3... 2... 1... loop!
    // loop through all the points in your object/scene/space
    // whatever - those points passed - so each is transformed
    for(int i = 7; i>=0;i-- )
    {
        // apply Math to making transformations
        // based on rotations

        // assign variables for the current x, y and z
        x = points[i].x;
        y = points[i].y;
        z = points[i].z;

        // perform the rotations around each axis
        // rotation around x
        xy = cx*y - sx*z;
        xz = sx*y + cx*z;
        // rotation around y
        yz = cy*xz - sy*x;
        yx = sy*xz + cy*x;
        // rotation around z
        zx = cz*yx - sz*xy;
        zy = sz*yx + cz*xy;

        // now determine perspective scaling factor
        // yz was the last calculated z value so its the
        // final value for z depth
        scaleRatio = focalLength/(focalLength + yz);
        // assign the new x and y
        x = zx*scaleRatio;
        y = zy*scaleRatio;
        z = yz;
        // create transformed 2D point with the calculated values
        // adding it to the array holding all 2D points
        TransformedPointsArray->push_back(Point2D((int)x, (int)y,(int)-z,scaleRatio));
    }
    // after looping return the array of points as they
    // exist after the rotation and scaling
}

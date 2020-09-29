
#include "GClipmap.h"
#include <math.h>
#include "CHgtFile.h"
#include "GLayer.h"

int n = 127;          //7, 15, 31, 63, 127, 255, 511, 1023
int m = (n + 1) / 4; //2,  4,  8, 16

GClipmap::GClipmap(COpenGl* openGlPointer) : openGl(openGlPointer)
{
    camera = openGl->drawingState.getCamera();
    
    program = new QOpenGLShaderProgram();
    vaoA = new QOpenGLVertexArrayObject;
    vaoB = new QOpenGLVertexArrayObject;
    vaoC = new QOpenGLVertexArrayObject;
    vaoD = new QOpenGLVertexArrayObject;
    vaoE = new QOpenGLVertexArrayObject;
    vaoF = new QOpenGLVertexArrayObject;

    initialized = false;     
}

void GClipmap::draw() {


    if (!initialized) {
        
        bool result;

        //program shader
        result = program->addShaderFromSourceFile(QOpenGLShader::Vertex, "VertexShader.txt");
        result = program->addShaderFromSourceFile(QOpenGLShader::Fragment, "FragmentShader.txt");
        program->link();
        program->bind();

        //getting aPos uniform location to insert data
        positionInputLocation = program->attributeLocation("position");
        HgtTextureLocation =    program->uniformLocation("tex_heightmap");
        pixelTextureLocation[0] =  program->uniformLocation("tex_pixelmap");
        pixelTextureLocation[1] =  program->uniformLocation("tex_pixelmap1");
        pixelTextureLocation[2] =  program->uniformLocation("tex_pixelmap2");
        pixelTextureLocation[3] =  program->uniformLocation("tex_pixelmap3");
        pixelTextureLocation[4] =  program->uniformLocation("tex_pixelmap4");
        pixelTextureLocation[5] =  program->uniformLocation("tex_pixelmap5");
        pixelTextureLocation[6] =  program->uniformLocation("tex_pixelmap6");
        pixelTextureLocation[7] =  program->uniformLocation("tex_pixelmap7");
        pixelTextureLocation[8] =  program->uniformLocation("tex_pixelmap8");
        pixelTextureLocation[9] =  program->uniformLocation("tex_pixelmap9");
        pixelTextureLocation[10] = program->uniformLocation("tex_pixelmap10");
        pixelTextureLocation[11] = program->uniformLocation("tex_pixelmap11");
        pixelTextureLocation[12] = program->uniformLocation("tex_pixelmap12");


        //initializing buffers
        initializeA_Buffer();
        initializeB_Buffer();
        initializeC_Buffer();
        initializeD_Buffer();
        initializeE_Buffer();
        initializeF_Buffer();

        //setting drawing mode
        //drawingMode = GL_TRIANGLE_STRIP;
        drawingMode = GL_TRIANGLE_STRIP;


        //building layers
       
                                      //degree   HgtfileDegree   scale  LOD  layerIndex  HgtSkipping   HgtRes   RawSkipping    RawRes
        layer.push_back(GLayer(this, 0.0018311,      3.75,          1,   13,        0,          2,      4097,        1,         24576, n));
        layer.push_back(GLayer(this, 0.0036621,      3.75,          2,   12,        1,          4,      4097,        2,         24576, n));
        layer.push_back(GLayer(this, 0.0073242,      3.75,          4,   11,        2,          8,      4097,        1,          6144, n));
        layer.push_back(GLayer(this, 0.0146484,      3.75,          8,   10,        3,         16,      4097,        2,          6144, n));
        layer.push_back(GLayer(this, 0.0292969,     15.00,         16,    9,        4,          1,       512,        4,          6144, n));
        layer.push_back(GLayer(this, 0.0585938,     15.00,         32,    8,        5,          2,       512,        1,           768, n));
        layer.push_back(GLayer(this, 0.1171876,     15.00,         64,    7,        6,          4,       512,        2,           768, n));
        layer.push_back(GLayer(this, 0.2343752,     15.00,        128,    6,        7,          8,       512,        4,           768, n));
        layer.push_back(GLayer(this, 0.4687504,     15.00,        256,    5,        8,         16,       512,        1,            96, n));

        
        //defining vertex array object

        
        //defining uniforms
        QVector2D worldScaleFactor;
        QVector2D worldOffset;
        QVector3D color;
        worldScaleFactor.setX(0.0018311);
        worldScaleFactor.setY(0.0018311);
        worldOffset.setX(0.0);
        worldOffset.setY(0.0);
        color.setX(0.0);
        color.setY(1.0);
        color.setZ(0.0);

        program->setUniformValue("worldScaleFactor", worldScaleFactor);
        program->setUniformValue("worldOffset", QVector2D(0.0, 0.0));
        program->setUniformValue("color", color);
        
        
        initialized = true; 
    }

    program->bind();
    program->setUniformValue("modelView", generateModelViewMatrix());
    

    //finding position
    findPosition();

    layer[0].setFillerPosition(0,0); //inner
    layer[1].setFillerPosition(1,1); //middle
    layer[2].setFillerPosition(0,0); //outer
    layer[3].setFillerPosition(0,0);
    layer[4].setFillerPosition(1,1);
    layer[5].setFillerPosition(1,1);
    layer[6].setFillerPosition(1,1);
    layer[7].setFillerPosition(0,0);
    layer[8].setFillerPosition(1,1);


    layer[0].setPosition(0,0);      //inner
    layer[1].setPosition(1,1);      //middle
    layer[2].setPosition(1,1);      //outer
    layer[3].setPosition(0,0);
    layer[4].setPosition(0,0);
    layer[5].setPosition(0,0);
    layer[6].setPosition(1,1);
    layer[7].setPosition(1,1);
    layer[8].setPosition(0,1);

    double treshold = 3000;
    
    if (distanceFromEarth < treshold)
        activeLevelOfDetail = 0;
    else if (distanceFromEarth < 2 * treshold)
        activeLevelOfDetail = 1;
    else if (distanceFromEarth < 4 * treshold)
        activeLevelOfDetail = 2;
    else if (distanceFromEarth < 8 * treshold)
        activeLevelOfDetail = 3;
    else if (distanceFromEarth < 16 * treshold)
        activeLevelOfDetail = 4;
    else if (distanceFromEarth < 32 * treshold)
        activeLevelOfDetail = 5;
    else if (distanceFromEarth < 64 * treshold)
        activeLevelOfDetail = 6;
    else if (distanceFromEarth < 128 * treshold)
        activeLevelOfDetail = 7;
    else
        activeLevelOfDetail = 8;
   
    activeLevelOfDetail = 8;
    for (int x = activeLevelOfDetail; x < 9; x++) 
        layer[x].buildLayer(tlon, tlat);
        
    program->release();
}

void GClipmap::findPosition() {

    double camX, camY, camZ;
    double lon, lat, rad;
    

    double earthRadius = CONST_EARTH_RADIUS;

    //finding current cameraToEarth point
    camera->getCamPosition(&camX, &camY, &camZ);

    distanceFromEarth = sqrt(camX * camX + camY * camY + camZ * camZ) - 6378100;

    //finding right tile
    CCommons::getSphericalFromCartesian(camX, camY, camZ, &lon, &lat, &rad);
    CCommons::findTopLeftCorner(lon, lat, 0.0018311, &tlon, &tlat);

}

void GClipmap::initializeA_Buffer() {
    //biding vertex array object
    vaoA->create();
    vaoA->bind();
        

    //vertex buffer
    vertexA_Buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexA_Buffer->create();
    vertexA_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexA_Buffer->bind();       //glBindBuffer
   
    QVector<float> verticesA;
    for (int y = 0; y < m; y++) 
        for(int x = 0; x < m; x++)
            verticesA << x << y;

    vertexA_Buffer->allocate(verticesA.constData(), verticesA.size() * sizeof(GLfloat));

    //index buffer
    indexA_Buffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indexA_Buffer->create();
    indexA_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexA_Buffer->bind();       //glBindBuffer

    QVector<GLint> indiciesA;

    for (int y = 0; y < m-1; y++) {

        if(!indiciesA.isEmpty())
            indiciesA.pop_back();

        if (y % 2 == 0) 
            for (float x = y*m; x < (y+1)*m; x++) 
                indiciesA << x << x + m;
        
        if (y % 2 == 1)
            for (float x = (y+1)*m-1; x > y*m-1; x--)
                indiciesA << x << x + m;

    }

    howManyToRenderA = indiciesA.size();

    program->setAttributeBuffer(positionInputLocation, GL_FLOAT, 0, 2, 0);
    program->enableAttributeArray(positionInputLocation);
    indexA_Buffer->allocate(indiciesA.constData(), indiciesA.size() * sizeof(GLint));
    indexA_Buffer->release();
    vaoA->release();
}

void GClipmap::initializeB_Buffer() {
    //biding vertex array object
    vaoB->create();
    vaoB->bind();

    //vertex buffer
    vertexB_Buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexB_Buffer->create();
    vertexB_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexB_Buffer->bind();       //glBindBuffer

    QVector<float> verticesB;
    for (int y = 0; y < m; y++)
        for (int x = 0; x < 3; x++)
            verticesB << x << y;

    vertexB_Buffer->allocate(verticesB.constData(), verticesB.size() * sizeof(GLfloat));

    //index buffer
    indexB_Buffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indexB_Buffer->create();
    indexB_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexB_Buffer->bind();       //glBindBuffer

    QVector<GLint> indiciesB;
    for (int y = 0; y < m-1; y++) {
        if (!indiciesB.isEmpty())
            indiciesB.pop_back();

        if (y % 2 == 0)
            for (int x = 0; x < 3; x++)
                indiciesB << x + y * 3 << x + y * 3 + 3;

        if (y % 2 == 1)
            for (int x = 2; x > -1; x--)
                indiciesB << x + y * 3 << x + y * 3 + 3;
    }

    howManyToRenderB = indiciesB.size();

    program->setAttributeBuffer(positionInputLocation, GL_FLOAT, 0, 2, 0);
    program->enableAttributeArray(positionInputLocation);
    indexB_Buffer->allocate(indiciesB.constData(), indiciesB.size() * sizeof(int));
    indexB_Buffer->release();
    vaoB->release();
}

void GClipmap::initializeC_Buffer() {
    //biding vertex array object
    vaoC->create();
    vaoC->bind();

    //vertex buffer
    vertexC_Buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexC_Buffer->create();
    vertexC_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexC_Buffer->bind();       //glBindBuffer

    QVector<float> verticesC;   
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < m; x++)
            verticesC << x << y;

    vertexC_Buffer->allocate(verticesC.constData(), verticesC.size() * sizeof(GLfloat));

    //index buffer
    indexC_Buffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indexC_Buffer->create();
    indexC_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexC_Buffer->bind();       //glBindBuffer

    QVector<GLint> indiciesC;
    for (int y = 0; y < 2; y++) {
        if (!indiciesC.isEmpty())
            indiciesC.pop_back();

        if (y % 2 == 0)
            for (int x = 0; x < m; x++)
                indiciesC << x << x + m;

        if (y % 2 == 1)
            for (int x = m - 1; x > -1; x--)
                indiciesC << x + m << x + 2 * m;
    }

    howManyToRenderC = indiciesC.size();

    program->setAttributeBuffer(positionInputLocation, GL_FLOAT, 0, 2, 0);
    program->enableAttributeArray(positionInputLocation);
    indexC_Buffer->allocate(indiciesC.constData(), indiciesC.size() * sizeof(int));
    indexC_Buffer->release();
    vaoB->release();
}

void GClipmap::initializeD_Buffer() {
    //biding vertex array object
    vaoD->create();
    vaoD->bind();


    //vertex buffer
    vertexD_Buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexD_Buffer->create();
    vertexD_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexD_Buffer->bind();       //glBindBuffer

    QVector<float> verticesD;
    for (float y = 0; y < 2; y++)
        for (float x = 0; x < (2*m+1); x++) //2,4,8,16
            verticesD << x << y;

    vertexD_Buffer->allocate(verticesD.constData(), verticesD.size() * sizeof(GLfloat));

    //index buffer
    indexD_Buffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indexD_Buffer->create();
    indexD_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexD_Buffer->bind();       //glBindBuffer

    QVector<GLint> indiciesD;
    for (float x = 0; x < (2*m+1); x++) //2,4,8,16
        indiciesD << x << x+2*m+1;

    howManyToRenderD = indiciesD.size();

    program->setAttributeBuffer(positionInputLocation, GL_FLOAT, 0, 2, 0);
    program->enableAttributeArray(positionInputLocation);
    indexD_Buffer->allocate(indiciesD.constData(), indiciesD.size() * sizeof(GLint));
    indexD_Buffer->release();
    vaoD->release();
}

void GClipmap::initializeE_Buffer() {
    //biding vertex array object
    vaoE->create();
    vaoE->bind();


    //vertex buffer
    vertexE_Buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexE_Buffer->create();
    vertexE_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexE_Buffer->bind();       //glBindBuffer

    QVector<float> verticesE;
    for (float y = 0; y < 2*m; y++)
        for (float x = 0; x < 2; x++)
            verticesE << x << y;

    vertexE_Buffer->allocate(verticesE.constData(), verticesE.size() * sizeof(GLfloat));

    //index buffer
    indexE_Buffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indexE_Buffer->create();
    indexE_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexE_Buffer->bind();       //glBindBuffer
    
    QVector<GLint> indiciesE;
    //indiciesE << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7
    //    << 8 << 9 << 10 << 11 << 12 << 13 << 14 << 15 << 16 << 17;
    for (int i = 0; i < 4 * m; i++)
        indiciesE << i;

    howManyToRenderE = indiciesE.size();

    program->setAttributeBuffer(positionInputLocation, GL_FLOAT, 0, 2, 0);
    program->enableAttributeArray(positionInputLocation);
    indexE_Buffer->allocate(indiciesE.constData(), indiciesE.size() * sizeof(GLint));
    indexE_Buffer->release();
    vaoE->release();
}

void GClipmap::initializeF_Buffer() {
    //biding vertex array object
    vaoF->create();
    vaoF->bind();


    //vertex buffer
    vertexF_Buffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexF_Buffer->create();
    vertexF_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vertexF_Buffer->bind();       //glBindBuffer

    QVector<float> verticesF;
    for (float y = 0; y < 2; y++)
        for (float x = 0; x < 2; x++)
            verticesF << x << y;

    vertexF_Buffer->allocate(verticesF.constData(), verticesF.size() * sizeof(GLfloat));

    //index buffer
    indexF_Buffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    indexF_Buffer->create();
    indexF_Buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexF_Buffer->bind();       //glBindBuffer

    QVector<GLint> indiciesF;
    indiciesF << 0 << 3 << 1 << 4 << 2 << 5 << 8 << 4
        << 7 << 3 << 6;

    program->setAttributeBuffer(positionInputLocation, GL_FLOAT, 0, 2, 0);
    program->enableAttributeArray(positionInputLocation);
    indexF_Buffer->allocate(indiciesF.constData(), indiciesF.size() * sizeof(GLint));
    indexF_Buffer->release();
    vaoF->release();
}

QMatrix4x4 GClipmap::generateModelViewMatrix() {
   
    QMatrix4x4 perspectiveMatrix;
    perspectiveMatrix.perspective(drawingStateSnapshot->camFOV, windowAspectRatio, zNear, zFar);

    QMatrix4x4 viewMatrix;
    viewMatrix.setToIdentity();
    QVector3D eye(drawingStateSnapshot->camPerspectiveX, drawingStateSnapshot->camPerspectiveY, drawingStateSnapshot->camPerspectiveZ);
    QVector3D center(-drawingStateSnapshot->camPerspectiveLookAtX, -drawingStateSnapshot->camPerspectiveLookAtY, -drawingStateSnapshot->camPerspectiveLookAtZ);
    QVector3D up(0.0, 1.0, 0.0);
    viewMatrix.lookAt(eye, center, up);

    QMatrix4x4 modelViewMatrix;
    return modelViewMatrix = perspectiveMatrix * viewMatrix;   //mvp is really a p*v*m matrix
}
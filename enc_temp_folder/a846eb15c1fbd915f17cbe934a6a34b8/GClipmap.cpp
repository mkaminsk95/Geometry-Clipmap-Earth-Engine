
#include "GClipmap.h"
#include <math.h>
#include "CHgtFile.h"
#include "GLayer.h"

int n = 255;          //7, 15, 31, 63, 127, 255, 511, 1023
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
    clipmapReady = false;

    activeLvlOfDetail = 8;
    highestLvlOfDetail = 8;
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
        heightTextureLocation[0] = program->uniformLocation("tex_heightmap0");
        heightTextureLocation[1] = program->uniformLocation("tex_heightmap1");
        heightTextureLocation[2] = program->uniformLocation("tex_heightmap2");
        heightTextureLocation[3] = program->uniformLocation("tex_heightmap3");
        heightTextureLocation[4] = program->uniformLocation("tex_heightmap4");
        heightTextureLocation[5] = program->uniformLocation("tex_heightmap5");
        heightTextureLocation[6] = program->uniformLocation("tex_heightmap6");
        heightTextureLocation[7] = program->uniformLocation("tex_heightmap7");
        heightTextureLocation[8] = program->uniformLocation("tex_heightmap8");
        heightTextureLocation[9] = program->uniformLocation("tex_heightmap9");
        heightTextureLocation[10] = program->uniformLocation("tex_heightmap10");
        heightTextureLocation[11] = program->uniformLocation("tex_heightmap11");
        heightTextureLocation[12] = program->uniformLocation("tex_heightmap12");

        pixelTextureLocation[0] =  program->uniformLocation("tex_pixelmap0");
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
        drawingMode = GL_LINE_STRIP;
        //drawingMode = GL_TRIANGLE_STRIP;


        //building layers

        layer.push_back(GLayer(this,  0.0018311,      3.75,          1,   0,          2,      4097,        1,         24576, n));  //0
        layer.push_back(GLayer(this,  0.0036621,      3.75,          2,   1,          4,      4097,        2,         24576, n));  //1
        layer.push_back(GLayer(this,  0.0073242,      3.75,          4,   2,          8,      4097,        1,          6144, n));  //2
        layer.push_back(GLayer(this,  0.0146484,      3.75,          8,   3,         16,      4097,        2,          6144, n));  //3
        layer.push_back(GLayer(this,  0.0292969,     15.00,         16,   4,          1,       513,        4,          6144, n));  //4
        layer.push_back(GLayer(this,  0.0585938,     15.00,         32,   5,          2,       513,        1,           768, n));  //5
        layer.push_back(GLayer(this,  0.1171876,     15.00,         64,   6,          4,       513,        2,           768, n));  //6
        layer.push_back(GLayer(this,  0.2343752,     15.00,        128,   7,          8,       513,        4,           768, n));  //7
        layer.push_back(GLayer(this,  0.4687504,     15.00,        256,   8,         16,       513,        1,            96, n));  //8 
        layer.push_back(GLayer(this,  0.9375008,     60.00,        512,   9,          1,        65,        2,            96, n));  //9
        layer.push_back(GLayer(this,  1.8750016,     60.00,       1024,  10,          2,        65,        4,            96, n));  //10
        layer.push_back(GLayer(this,  3.7500032,     60.00,       2048,  11,          4,        65,        8,            96, n));  //11
        layer.push_back(GLayer(this,  7.5000064,     60.00,       4096,  12,          8,        65,       16,            96, n));  //12

        layer[0].setPosition(1, 0);
        layer[1].setPosition(1, 0);
        layer[2].setPosition(1, 0);
        layer[3].setPosition(1, 0);
        layer[4].setPosition(1, 0);
        layer[5].setPosition(1, 0);
        layer[6].setPosition(1, 0);
        layer[7].setPosition(1, 0);
        layer[8].setPosition(1, 0);
        layer[9].setPosition(1, 0);
        layer[10].setPosition(1, 0);
        layer[11].setPosition(1, 0);

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
        program->setUniformValue("n", n);
        
        initialized = true; 

        openGl->clipmapThread->start();
        
    }

    program->bind();
    program->setUniformValue("modelView", generateModelViewMatrix());
    

    if(clipmapReady) {

        for (int x = activeLvlOfDetail; x <= highestLvlOfDetail; x++)
            layer[x].updateTextures();

        activeLvlOfDetail = openGl->clipmapThread->activeLvlOfDetail;
        highestLvlOfDetail = openGl->clipmapThread->highestLvlOfDetail;
        clipmapReady = false;
    }

    for (int x = activeLvlOfDetail; x <= highestLvlOfDetail; x++)
        layer[x].buildLayer();
    

    program->release();
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
    QVector3D center(drawingStateSnapshot->camPerspectiveLookAtX, drawingStateSnapshot->camPerspectiveLookAtY, drawingStateSnapshot->camPerspectiveLookAtZ);
    //QVector3D center(-100000000, -1000000, 1000000);
    QVector3D up(0.0, 1.0, 0.0);
   /* CCommons::doubleIntoVSConsole(-drawingStateSnapshot->camPerspectiveLookAtX);
    CCommons::doubleIntoVSConsole(-drawingStateSnapshot->camPerspectiveLookAtY);
    CCommons::doubleIntoVSConsole(-drawingStateSnapshot->camPerspectiveLookAtZ);
    CCommons::stringIntoVSConsole("\n");*/
    viewMatrix.lookAt(eye, center, up);

    QMatrix4x4 modelViewMatrix;
    return modelViewMatrix = perspectiveMatrix * viewMatrix;   //mvp is really a p*v*m matrix
}
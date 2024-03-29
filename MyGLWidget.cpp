#include <GL/glew.h>
#include "MyGLWidget.h"

#include <iostream>

MyGLWidget::MyGLWidget (QGLFormat &f, QWidget* parent) : QGLWidget(f, parent)
{
  setFocusPolicy(Qt::ClickFocus);  // per rebre events de teclat
  xClick = yClick = 0;
  angleY = /*M_PI*3/4.*/ M_PI*7/4.;
  angleX = /*M_PI/8.*/ -M_PI/8.;
  DoingInteractive = NONE;
  radiAux = sqrt(200) + 1; //fer diagonal per calcular radi, calcular hipotenusa sqrt(10^2 + 10^2)
  //el +1 es perquè el sqrt(200) és la mida del terra pero no conta amb el volum del lego i el Patricio
  ra = 1.0;
  fovAngle = M_PI/3.0;
  colFocus = glm::vec3(0.8, 0.8, 0.8); // en SCA
  posFocus = glm::vec3(0, 2, 0);  // en SCA
  posObs = glm::vec3(0, 0, -2*radiAux);
  vrp = glm::vec3(0,0,0);
  up = glm::vec3(0,1,0);
  isCameraModified = false;
  zNear = double(radiAux);
  zFar = 3. * double(radiAux);
  isFocusRed = false;
  isOrthoPlantCamera = false;
  left = -11.;
  right = 11.;
  bottom = -11.;
  top = 11.;
  angleModelsRotation = 0.;
}

void MyGLWidget::initializeGL ()
{
  // glew és necessari per cridar funcions de les darreres versions d'OpenGL
  glewExperimental = GL_TRUE;
  glewInit(); 
  glGetError();  // Reinicia la variable d'error d'OpenGL

  glClearColor(0.7, 0.7, 1.0, 1.0);  // defineix color de fons (d'esborrat)
  glEnable(GL_DEPTH_TEST);
  loadShaders();
  createBuffers();
  projectTransform();
  viewTransform();
}

void MyGLWidget::paintGL ()
{
  //serveix per inicialitzar el focus
  focusTransform();

  // Esborrem el frame-buffer i el depth-buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Activem el VAO per a pintar el terra 
  glBindVertexArray (VAO_Terra);

  modelTransformTerra ();

  // pintem
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // Activem el VAO per a pintar el Patricio
  glBindVertexArray (VAO_Patr);

  modelTransformPatricio ();

  // Pintem l'escena
  glDrawArrays(GL_TRIANGLES, 0, patr.faces().size()*3);

  //lego
  glBindVertexArray (VAO_lego);

  modelTransformLego();

  // Pintem l'escena
  glDrawArrays(GL_TRIANGLES, 0, lego.faces().size()*3);

  
  glBindVertexArray(0);
}

void MyGLWidget::resizeGL (int w, int h)
{
  ra = double(w) / double(h);
  if (ra >= 1) projectTransform();
  else {
      auxAngle = fovAngle;
      fovAngle = 2 * atan(tan(auxAngle / 2) / ra);
      projectTransform();
      fovAngle = auxAngle;
  }
  glViewport (0, 0, w, h);
}

void MyGLWidget::createBuffers ()
{
  // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
  patr.load("./models/Patricio.obj");

  // Calculem la capsa contenidora del model
  calculaCapsaModel ();
  
  // Creació del Vertex Array Object del Patricio
  glGenVertexArrays(1, &VAO_Patr);
  glBindVertexArray(VAO_Patr);

  // Creació dels buffers del model patr
  // Buffer de posicions
  glGenBuffers(1, &VBO_PatrPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_vertices(), GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  // Buffer de normals
  glGenBuffers(1, &VBO_PatrNorm);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrNorm);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_normals(), GL_STATIC_DRAW);

  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glGenBuffers(1, &VBO_PatrMatamb);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrMatamb);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matamb(), GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glGenBuffers(1, &VBO_PatrMatdiff);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrMatdiff);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matdiff(), GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glGenBuffers(1, &VBO_PatrMatspec);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrMatspec);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3*3, patr.VBO_matspec(), GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glGenBuffers(1, &VBO_PatrMatshin);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_PatrMatshin);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*patr.faces().size()*3, patr.VBO_matshin(), GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);


  //LEGOMAN

  // Carreguem el model de l'OBJ - Atenció! Abans de crear els buffers!
  lego.load("./models/legoman.obj");

    // Calculem la capsa contenidora del model
    calculaCapsaCirc ();

    // Creació del Vertex Array Object del Legoman
    glGenVertexArrays(1, &VAO_lego);
    glBindVertexArray(VAO_lego);

    // Creació dels buffers del model lego
    // Buffer de posicions
    glGenBuffers(1, &VBO_legoPos);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_legoPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*lego.faces().size()*3*3, lego.VBO_vertices(), GL_STATIC_DRAW);

    // Activem l'atribut vertexLoc
    glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(vertexLoc);

    // Buffer de normals
    glGenBuffers(1, &VBO_legoNorm);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_legoNorm);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*lego.faces().size()*3*3, lego.VBO_normals(), GL_STATIC_DRAW);

    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(normalLoc);

    // En lloc del color, ara passem tots els paràmetres dels materials
    // Buffer de component ambient
    glGenBuffers(1, &VBO_legoMatamb);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_legoMatamb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*lego.faces().size()*3*3, lego.VBO_matamb(), GL_STATIC_DRAW);

    glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matambLoc);

    // Buffer de component difusa
    glGenBuffers(1, &VBO_legoMatdiff);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_legoMatdiff);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*lego.faces().size()*3*3, lego.VBO_matdiff(), GL_STATIC_DRAW);

    glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matdiffLoc);

    // Buffer de component especular
    glGenBuffers(1, &VBO_legoMatspec);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_legoMatspec);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*lego.faces().size()*3*3, lego.VBO_matspec(), GL_STATIC_DRAW);

    glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matspecLoc);

    // Buffer de component shininness
    glGenBuffers(1, &VBO_legoMatshin);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_legoMatshin);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*lego.faces().size()*3, lego.VBO_matshin(), GL_STATIC_DRAW);

    glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(matshinLoc);


  // Dades del terra

  // VBO amb la posició dels vèrtexs
  glm::vec3 posterra[4] = {
	glm::vec3( 10.0, 0.0, -10.0),
	glm::vec3( 10.0, 0.0,  10.0),
	glm::vec3(-10.0, 0.0, -10.0),
	glm::vec3(-10.0, 0.0,  10.0)
  }; 

  // VBO amb la normal de cada vèrtex
  glm::vec3 norm (0,1,0);
  glm::vec3 normterra[4] = {
	norm, norm, norm, norm
  };

  // Definim el material del terra
  glm::vec3 amb(0.1,0.09,0.);
  glm::vec3 diff(0.,0.8,0.);
  glm::vec3 spec(1.,1.,1.);
  float shin = 100;

  // Fem que aquest material afecti a tots els vèrtexs per igual
  glm::vec3 matambterra[4] = {
	amb, amb, amb, amb
  };
  glm::vec3 matdiffterra[4] = {
	diff, diff, diff, diff
  };
  glm::vec3 matspecterra[4] = {
	spec, spec, spec, spec
  };
  float matshinterra[4] = {
	shin, shin, shin, shin
  };

// Creació del Vertex Array Object del terra
  glGenVertexArrays(1, &VAO_Terra);
  glBindVertexArray(VAO_Terra);

  glGenBuffers(1, &VBO_TerraPos);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraPos);
  glBufferData(GL_ARRAY_BUFFER, sizeof(posterra), posterra, GL_STATIC_DRAW);

  // Activem l'atribut vertexLoc
  glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(vertexLoc);

  glGenBuffers(1, &VBO_TerraNorm);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraNorm);
  glBufferData(GL_ARRAY_BUFFER, sizeof(normterra), normterra, GL_STATIC_DRAW);

  // Activem l'atribut normalLoc
  glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(normalLoc);

  // En lloc del color, ara passem tots els paràmetres dels materials
  // Buffer de component ambient
  glGenBuffers(1, &VBO_TerraMatamb);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraMatamb);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matambterra), matambterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matambLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matambLoc);

  // Buffer de component difusa
  glGenBuffers(1, &VBO_TerraMatdiff);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraMatdiff);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matdiffterra), matdiffterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matdiffLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matdiffLoc);

  // Buffer de component especular
  glGenBuffers(1, &VBO_TerraMatspec);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraMatspec);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matspecterra), matspecterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matspecLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matspecLoc);

  // Buffer de component shininness
  glGenBuffers(1, &VBO_TerraMatshin);
  glBindBuffer(GL_ARRAY_BUFFER, VBO_TerraMatshin);
  glBufferData(GL_ARRAY_BUFFER, sizeof(matshinterra), matshinterra, GL_STATIC_DRAW);

  glVertexAttribPointer(matshinLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(matshinLoc);

  glBindVertexArray(0);
}

void MyGLWidget::loadShaders ()
{
  // Creem els shaders per al fragment i vertex shader
  QGLShader fs(QGLShader::Fragment, this);
  QGLShader vs(QGLShader::Vertex, this);
  // Carreguem el codi dels fitxers i els compilem 
  fs.compileSourceFile("./shaders/fragshad.frag");
  vs.compileSourceFile("./shaders/vertshad.vert");

  // Creem el program
  program = new QGLShaderProgram(this);
  // Li afegim els shaders corresponents
  program->addShader(&fs);
  program->addShader(&vs);

  // Linkem el program
  program->link();

  // Indiquem que aquest és el program que volem usar
  program->bind();

  // Obtenim identificador per a l'atribut “vertex” del vertex shader
  vertexLoc = glGetAttribLocation (program->programId(), "vertex");
  // Obtenim identificador per a l'atribut “normal” del vertex shader
  normalLoc = glGetAttribLocation (program->programId(), "normal");
  // Obtenim identificador per a l'atribut “matamb” del vertex shader
  matambLoc = glGetAttribLocation (program->programId(), "matamb");
  // Obtenim identificador per a l'atribut “matdiff” del vertex shader
  matdiffLoc = glGetAttribLocation (program->programId(), "matdiff");
  // Obtenim identificador per a l'atribut “matspec” del vertex shader
  matspecLoc = glGetAttribLocation (program->programId(), "matspec");
  // Obtenim identificador per a l'atribut “matshin” del vertex shader
  matshinLoc = glGetAttribLocation (program->programId(), "matshin");

  // Demanem identificadors per als uniforms del vertex shader
  transLoc = glGetUniformLocation (program->programId(), "TG");
  projLoc = glGetUniformLocation (program->programId(), "proj");
  viewLoc = glGetUniformLocation (program->programId(), "view");
  colFocusLoc = glGetUniformLocation (program->programId(), "colFocus");
  posFocusLoc = glGetUniformLocation (program->programId(), "posFocus");
}

void MyGLWidget::focusTransform()
{
    glUniform3f (colFocusLoc, colFocus.x, colFocus.y, colFocus.z);
    glUniform3f (posFocusLoc, posFocus.x, posFocus.y, posFocus.z);
}

void MyGLWidget::modelTransformPatricio ()
{
  glm::mat4 TG;  // Matriu de transformació
  TG = glm::mat4(1.f);
  TG = glm::rotate(TG, angleModelsRotation, glm::vec3(0,1,0)); //Afegit per fer rotació pitjant R
  TG = glm::translate(TG, glm::vec3(-10 , 0, -10));
  TG = glm::scale(TG, glm::vec3(scalePatr, scalePatr, scalePatr));
  TG = glm::rotate(TG, (float)M_PI/4, glm::vec3(0,1,0));
  TG = glm::translate(TG, -centreBasePatr);

  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::modelTransformLego ()
{
  glm::mat4 TG;  // Matriu de transformació
  TG = glm::mat4(1.f);
  TG = glm::rotate(TG, angleModelsRotation, glm::vec3(0,1,0)); //Afegit per fer rotació pitjant R
  TG = glm::translate(TG, glm::vec3(-10 ,0, 10));
  TG = glm::rotate(TG, (float)M_PI*3/4, glm::vec3(0,1,0));
  TG = glm::scale(TG, glm::vec3(scaleLego, scaleLego, scaleLego));
  TG = glm::translate(TG, -centreBaselego);

  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}


void MyGLWidget::modelTransformTerra ()
{
  glm::mat4 TG;  // Matriu de transformació
  TG = glm::mat4(1.f);
  glUniformMatrix4fv (transLoc, 1, GL_FALSE, &TG[0][0]);
}

void MyGLWidget::projectTransform ()
{
  glm::mat4 Proj;  // Matriu de projecció
//  Proj = glm::perspective(M_PI/3.0, 1.0, double(radiAux), 3.*double(radiAux));
  if (isOrthoPlantCamera) Proj = glm::ortho(left, right, bottom, top, zNear, zFar);
  else Proj = glm::perspective(fovAngle, ra, zNear, zFar);
  //FOV = 60, ra (rati aspecte), zNear, zFar
  glUniformMatrix4fv (projLoc, 1, GL_FALSE, &Proj[0][0]);
}

void MyGLWidget::viewTransform ()
{
  glm::mat4 View;  // Matriu de posició i orientació
//  View = glm::translate(glm::mat4(1.f), posObs); //OBS = -2*R
  if (isOrthoPlantCamera) View = glm::lookAt(glm::vec3(0,2*radiAux,0), vrp, glm::vec3(0,0,1));    //VISTA EN PLANTA
  else {
  View = glm::lookAt(posObs, vrp, up);
  //lookAt(OBS, VRP, UP)
  View = glm::rotate(View, angleX, glm::vec3(1, 0, 0));
  View = glm::rotate(View, -angleY, glm::vec3(0, 1, 0));
  }
  glUniformMatrix4fv (viewLoc, 1, GL_FALSE, &View[0][0]);
}


void MyGLWidget::calculaCapsaModel ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = patr.vertices()[0];
  miny = maxy = patr.vertices()[1];
  minz = maxz = patr.vertices()[2];
  for (unsigned int i = 3; i < patr.vertices().size(); i+=3)
  {
    if (patr.vertices()[i+0] < minx)
      minx = patr.vertices()[i+0];
    if (patr.vertices()[i+0] > maxx)
      maxx = patr.vertices()[i+0];
    if (patr.vertices()[i+1] < miny)
      miny = patr.vertices()[i+1];
    if (patr.vertices()[i+1] > maxy)
      maxy = patr.vertices()[i+1];
    if (patr.vertices()[i+2] < minz)
      minz = patr.vertices()[i+2];
    if (patr.vertices()[i+2] > maxz)
      maxz = patr.vertices()[i+2];
  }
  scalePatr = 3/(maxy-miny);
  centreBasePatr[0] = (minx+maxx)/2.0; centreBasePatr[1] = miny; centreBasePatr[2] = (minz+maxz)/2.0;
}

void MyGLWidget::calculaCapsaCirc ()
{
  // Càlcul capsa contenidora i valors transformacions inicials
  float minx, miny, minz, maxx, maxy, maxz;
  minx = maxx = lego.vertices()[0];
  miny = maxy = lego.vertices()[1];
  minz = maxz = lego.vertices()[2];
  for (unsigned int i = 3; i < lego.vertices().size(); i+=3)
  {
    if (lego.vertices()[i+0] < minx)
      minx = lego.vertices()[i+0];
    if (lego.vertices()[i+0] > maxx)
      maxx = lego.vertices()[i+0];
    if (lego.vertices()[i+1] < miny)
      miny = lego.vertices()[i+1];
    if (lego.vertices()[i+1] > maxy)
      maxy = lego.vertices()[i+1];
    if (lego.vertices()[i+2] < minz)
      minz = lego.vertices()[i+2];
    if (lego.vertices()[i+2] > maxz)
      maxz = lego.vertices()[i+2];
  }
  scaleLego = 1/(maxz-minz);
  centreBaselego[0] = (minx+maxx)/2.0; centreBaselego[1] = miny; centreBaselego[2] = (minz+maxz)/2.0;
}

void MyGLWidget::keyPressEvent (QKeyEvent *e)
{
  switch (e->key())
  {
    case Qt::Key_C:
      {
        if(isCameraModified){
            posObs = glm::vec3(0, 0, -2*radiAux);
            vrp = glm::vec3(0 ,0, 0);
            up = glm::vec3(0,1,0);
            zNear = double(radiAux);
            zFar = 3. * double(radiAux);
            angleY = M_PI*7/4.;
            angleX = -M_PI/8.;
            //aqui es podria afegir angleXaux i angleYaux per recuperar la càmera
        } else {
            posObs = glm::vec3(-10 , 4, -10);
            vrp = glm::vec3(-10 ,0, 10);
            up = glm::vec3(0,1,0);
            zNear = 1;
            zFar = sqrt(16 + 4000);
            angleX = 0.;
            angleY = 0.;
        }
        isCameraModified = !isCameraModified;
        projectTransform();
        viewTransform();
        break;
      }

    case Qt::Key_L:
      {
        if (isFocusRed){
            colFocus = glm::vec3(0.8, 0.8, 0.8);
        } else {
            colFocus = glm::vec3(0.8, 0., 0.);
        }
        isFocusRed = !isFocusRed;
        break;
      }

    //camera axonomètrica vista en planta
    case Qt::Key_A:
    {
        isOrthoPlantCamera = !isOrthoPlantCamera;
        projectTransform();
        viewTransform();
        break;
    }

    case Qt::Key_R:
    {
        angleModelsRotation += M_PI/8.0;
        break;
    }

    case Qt::Key_Escape:
        exit(0);
    default: e->ignore(); break;
  }
  updateGL();
}

void MyGLWidget::mousePressEvent (QMouseEvent *e)
{
  xClick = e->x();
  yClick = e->y();

  if (e->button() & Qt::LeftButton &&
      ! (e->modifiers() & (Qt::ShiftModifier|Qt::AltModifier|Qt::ControlModifier)))
  {
    DoingInteractive = ROTATE;
  }
}

void MyGLWidget::mouseReleaseEvent( QMouseEvent *)
{
  DoingInteractive = NONE;
}

void MyGLWidget::mouseMoveEvent(QMouseEvent *e)
{
  // Aqui cal que es calculi i s'apliqui la rotacio o el zoom com s'escaigui...
  if (DoingInteractive == ROTATE && !isCameraModified)
  {
    // Fem la rotació
    angleY += (e->x() - xClick) * M_PI / 180.0;
    angleX += (yClick - e->y()) * M_PI / 180.0;
    viewTransform ();
  }

  xClick = e->x();
  yClick = e->y();

  updateGL ();
}



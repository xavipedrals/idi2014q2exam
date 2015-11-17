#version 330 core

in vec3 vertex;
in vec3 normal;

in vec3 matamb;
in vec3 matdiff;
in vec3 matspec;
in float matshin;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 TG;

// Valors per als components que necessitem dels focus de llum
uniform vec3 colFocus;
vec3 llumAmbient = vec3(0.2, 0.2, 0.2);
uniform vec3 posFocus;

out vec3 fcolor;

//Lambert no té en compte els reflexes, només la llum difusa
vec3 Lambert (vec3 NormSCO, vec3 L)
{
    // S'assumeix que els vectors que es reben com a paràmetres estan normalitzats

    // Inicialitzem color a component ambient
    vec3 colRes = llumAmbient * matamb;

    // Afegim component difusa, si n'hi hax
    if (dot (L, NormSCO) > 0)
      colRes = colRes + colFocus * matdiff * dot (L, NormSCO);
    return (colRes);
}

//Phong té en compte els reflexes, és la llum difusa i especular
vec3 Phong (vec3 NormSCO, vec3 L, vec4 vertSCO)
{
    // Els vectors estan normalitzats

    // Inicialitzem color a Lambert
    vec3 colRes = Lambert (NormSCO, L);

    // Calculem R i V
    if (dot(NormSCO,L) < 0)
      return colRes;  // no hi ha component especular

    vec3 R = reflect(-L, NormSCO); // equival a: normalize (2.0*dot(NormSCO,L)*NormSCO - L);
    vec3 V = normalize(-vertSCO.xyz);

    if ((dot(R, V) < 0) || (matshin == 0))
      return colRes;  // no hi ha component especular

    // Afegim la component especular
    float shine = pow(max(0.0, dot(R, V)), matshin);
    return (colRes + matspec * colFocus * shine);
}

void main()
{
    vec3 normalSCO1; //normal del vertex en coordenades d'obs

    //Passem el vector a normal SCO (multiplicant per la matriu inversa de la transposada de view*TG)
    mat3 normalMatrix = inverse ( transpose (mat3 (view*TG) ) );
    normalSCO1 = normalize(normalMatrix*normal);


    vec3 L;//posició focus de llum en coordenades d'obs

    //passem la posició del focus a coordenades d'obs
    vec4 posFocusSCO = /*view * */ vec4 (posFocus, 1.0); //si deixes de multiplicar per view poses la camera en posicioOBS
    //passem la posició del vertex a coordenades d'obs
    vec4 vertexSCO = view * TG * vec4 (vertex, 1.0);

    //calculem L fent la distancia entre el focus i el vertex
    L = posFocusSCO.xyz - vertexSCO.xyz;
    L = normalize(L);

    //fcolor = matdiff;

    fcolor = Phong ( normalSCO1 , L, vertexSCO );
    //fcolor = Lambert ( normalSCO1 , L );
    gl_Position = proj * view * TG * vec4 (vertex, 1.0);
}

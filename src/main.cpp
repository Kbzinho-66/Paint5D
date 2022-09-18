#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <stack>
#include <sstream>
#include <unistd.h>
#include <time.h>
#include <vector>

// Nome dos arquivos a serem utilizados / gerados pelo programa.
std::string FILENAME_IN, FILENAME_OUT;
int fileSeq = 0;

// LOGS de saída do programa.
std::string ROOT_LOG = "\033[1;33mROOT: \033[0m";
std::string ERROR_LOG = "\033[1;31mERROR: \033[0m";
std::string RUNTIME_LOG = "\033[0;32mRUNTIME: \033[0m";
std::string FUNCTION_LOG = "\033[0;35mFUNCTION: \033[0m";
std::string FUNCTION_ARGS_LOG = "\033[0;35mFUNCTION_ARGS: \033[0m";

// Estrutura para representar pontos.
typedef struct {int x, y;} Point;

// Variáveis necessárias para o SDL.
unsigned int * pixels;
int width, height;
SDL_Surface * window_surface;
SDL_Surface * image;
SDL_Renderer * renderer;
bool ctrlState = false;
std::vector<Point> points;

std::string titulo = "Paint5D ";

// Valores RGB para a cor de fundo da janela.
const int VERMELHO = 255;
const int VERDE = 255;
const int AZUL = 255;

// Funções de desenho.
enum class Function {None, Line, Rectangle, Polygon, Circle, Bezier, Bucket};
Function f;

/**************************************** FUNÇÕES BÁSICAS ****************************************/

// Gera uma estrutura Point a partir de valores para x e y.
Point getPoint(int x, int y) {
    Point p;
    p.x = x;
    p.y = y;
    return p;
}

// Obtém a cor de um pixel de uma determinada posição.
Uint32 getPixel(int x, int y) {
    if((x>=0 && x<=width) && (y>=0 && y<=height))
        return pixels[x + width * y];
    else
        return -1;
}

// Seta um pixel em uma determinada posição, através da coordenadas de cor r, g, b, e alpha (transparência)
// r, g, b e a variam de 0 até 255.
void setPixel(int x, int y, int r, int g, int b, int a) {
    pixels[x + y * width] = SDL_MapRGBA(window_surface->format, r, g, b, a);
}

// Seta um pixel em uma determinada posição, através da coordenadas de cor r, g e b
// r, g, e b variam de 0 até 255.
void setPixel(int x, int y, int r, int g, int b) {
    setPixel(x, y, r, g, b, 255);
}

// Seta um pixel em uma determinada posição através das coordenadas de cor r, g e b
// r, g, e b variam de 0 até 255.
void setPixel(Point p, int r, int g, int b) {
    setPixel(p.x, p.y, r, g, b, 255);
}

// Seta um pixel em uma determinada posição, através de um Uint32 representando uma cor RGB.
void setPixel(int x, int y, Uint32 color) {
    if((x<0 || x>=width || y<0 || y>=height)) {
        return;
    }
    pixels[x + y * width] = color;
}

// Seta um pixel em uma determinada posição,
// através de um Uint32 representando
// uma cor RGB.
void setPixel(Point p, Uint32 color) {
    int x, y;
    x = p.x;
    y = p.y;

    if((x<0 || x>=width || y<0 || y>=height)) {
        return;
    }
    pixels[x + y * width] = color;
}

// Mostra na barra de título da janela a posição atual do mouse.
void showMousePosition(SDL_Window * window, int x, int y) {
    std::stringstream ss;
    ss << titulo << " X: " << x << " Y: " << y;
    SDL_SetWindowTitle(window, ss.str().c_str());
}

// Imprime na console a posição atual do mouse.
void printMousePosition(int x, int y) {
    printf("Mouse on x = %d, y = %d\n",x,y);
}

// Retorna uma cor RGB(UInt32) formada pelas componentes r, g, b e a(transparência) informadas
// r, g, b e a variam de 0 até 255.
Uint32 RGB(int r, int g, int b, int a) {
    return SDL_MapRGBA(window_surface->format, r, g, b, a);
}

// Retorna uma cor RGB(UInt32) formada pelas componentes r, g, e b informadas
// r, g e b variam de 0 até 255, a transparência é sempre 255 (imagem opaca).
Uint32 RGB(int r, int g, int b) {
    return SDL_MapRGBA(window_surface->format, r, g, b, 255);
}

// Retorna um componente de cor de uma cor RGB informada
// aceita os parâmetros 'r', 'R','g', 'G','b' e 'B'.
Uint8 getColorComponent( Uint32 pixel, char component ) {
    Uint32 mask;

    switch(component) {
        case 'b' :
        case 'B' :
            mask = RGB(0,0,255);
            pixel = pixel & mask;
            break;
        case 'r' :
        case 'R' :
            mask = RGB(255,0,0);
            pixel = pixel & mask;
            pixel = pixel >> 16;
            break;
        case 'g' :
        case 'G' :
            mask = RGB(0,255,0);
            pixel = pixel & mask;
            pixel = pixel >> 8;
            break;
    }
    
    return (Uint8) pixel;
}

int iPartOfNumber(float x) {
    return (int) x;
}

float fPartOfNumber(float x) {
    return iPartOfNumber(x + 0.5);
}

float rfPartOfNumber(float x) {
    return 1 - fPartOfNumber(x);
}

// Desenha todas as 8 coordenadas de (x, y), uma para cada octante.
void displayBresenhamCircle(int xc, int yc, int x, int y, Uint32 color) {
    setPixel(xc+x, yc+y, color);
    setPixel(xc-x, yc+y, color);
    setPixel(xc+x, yc-y, color);
    setPixel(xc-x, yc-y, color);
    setPixel(xc+y, yc+x, color);
    setPixel(xc-y, yc+x, color);
    setPixel(xc+y, yc-x, color);
    setPixel(xc-y, yc-x, color);
}

int euclideanDistance(Point p1, Point p2) {

    int x1, y1, x2, y2;
    x1 = p1.x;
    y1 = p1.y;
    x2 = p2.x;
    y2 = p2.y;

    double dist = pow(x2 - x1, 2) + pow(y2 - y1, 2);
    dist = sqrt(dist);

    return (int) dist;
}

/**************************************** FUNÇÕES BÁSICAS ****************************************/

/**************************************** FUNÇÕES DE DESENHO ****************************************/

// Preenche uma região com uma cor partindo de um ponto (x,y) enquanto houver pixels da cor original.
void floodFill(int x, int y, Uint32 newColor, Uint32 oldColor) {

    if (x < 0 || x > width - 1 || y < 0 || y > height - 1) {
        return;
    }

    std::stack<Point> st;
    st.push(getPoint(x, y));

    while (st.size() > 0) {
        Point p = st.top();
        st.pop();

        x = p.x;
        y = p.y;
        if (x < 0 || x > width - 1 || y < 0 || y > height - 1) {
            continue;
        }

        if (getPixel(x, y) == oldColor) {
            setPixel(x, y, newColor);
            st.push(getPoint(x-1, y));
            st.push(getPoint(x+1, y));
            st.push(getPoint(x, y-1));
            st.push(getPoint(x, y+1));
        }
    }
}

void floodFill(int x, int y, Uint32 color) {
    Uint32 oldColor = getPixel(x, y);
    floodFill(x, y, color, oldColor);
}

// Preenche uma região com uma cor partindo de um ponto enquanto houver pixels da cor original.
void floodFill(Point p, Uint32 newColor, Uint32 oldColor) {
    int x, y;
    x = p.x;
    y = p.y;
    floodFill(x, y, newColor, oldColor);
}

// Preenche uma região indefinida com uma cor partindo de um ponto.
void floodFill(Point p, Uint32 color) {
    Uint32 oldColor = getPixel(p.x, p.y);
    floodFill(p, color, oldColor);
}

// Desenha uma linha de Bresenham a partir de 4 coordenadas e uma cor.
void bresenhamLine(int x1, int y1, int x2, int y2, Uint32 cor) {
    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;

    dx=x2-x1;
    dy=y2-y1;

    dx1=fabs(dx);
    dy1=fabs(dy);

    px=2*dy1-dx1;
    py=2*dx1-dy1;

    if(dy1<=dx1) {
        if(dx>=0) {
            x=x1;
            y=y1;
            xe=x2;
        } else {
            x=x2;
            y=y2;
            xe=x1;
        }
        setPixel(x,y,cor);
        for(i=0;x<xe;i++) {
            x=x+1;
            if(px<0) {
                px=px+2*dy1;
            } else {
                if((dx<0 && dy<0) || (dx>0 && dy>0)) {
                    y=y+1;
                } else {
                    y=y-1;
                }
                px=px+2*(dy1-dx1);
            }
            setPixel(x,y,cor);
        }
    } else {
        if(dy>=0) {
            x=x1;
            y=y1;
            ye=y2;
        } else {
            x=x2;
            y=y2;
            ye=y1;
        }
        setPixel(x,y,cor);
        for(i=0;y<ye;i++) {
            y=y+1;
            if(py<=0) {
                py=py+2*dx1;
            } else {
                if((dx<0 && dy<0) || (dx>0 && dy>0)) {
                    x=x+1;
                } else {
                    x=x-1;
                }
                py=py+2*(dx1-dy1);
            }
            setPixel(x,y,cor);
        }
    }
}

// Desenha uma linha de Bresenham com base em 2 pontos e uma cor.
void bresenhamLine(Point p1, Point p2, Uint32 color) {
    bresenhamLine(p1.x, p1.y, p2.x, p2.y, color);
}

// Desenha um retângulo com o canto superior esquerdo em (x1, y1) e canto inferior direito em (x2, y2).
void rectangle(int x1, int y1, int x2, int y2, Uint32 color, bool fill = false) {
    bresenhamLine(x1, y1, x2, y1, color);
    bresenhamLine(x1, y1, x1, y2, color);
    bresenhamLine(x1, y2, x2, y2, color);
    bresenhamLine(x2, y1, x2, y2, color);

    if (fill) {
        floodFill(x1 + 1, y1 + 1, color);
    }
}

// Desenha um retângulo com base no ponto superior esquerdo, no inferior direito e em uma cor.
void rectangle(Point p1, Point p2, Uint32 color, bool fill = false) {
    rectangle(p1.x, p1.y, p2.x, p2.y, color, fill);
}

// Desenha um círculo de Bresenham com base em duas coordenadas para o ponto central,
// um valor para o raio e uma cor.
void bresenhamCircle(int xc, int yc, int radius, Uint32 color) {
    int x = 0;
    int y = radius;
    int decisionParameter = 3 - 2 * radius;

    displayBresenhamCircle(xc, yc, x, y, color);

    while (y >= x) {
        x++;

        if (decisionParameter > 0) {
            y--;
            decisionParameter = decisionParameter + 4 * (x - y) + 10;
        } else {
            decisionParameter = decisionParameter + 4 * x + 6;
        }

        displayBresenhamCircle(xc, yc, x, y, color);
    }

}

// Desenha um círculo de Bresenham com base em um Ponto central, um raio e uma cor.
void bresenhamCircle(Point p, int radius, Uint32 color) {
    int xc = p.x;
    int yc = p.y;
    bresenhamCircle(xc, yc, radius, color);
}

// Desenha uma curva de Bézier com base em 4 pontos e uma cor.
// p.at(0) e p.at(3) são os pontos extremos da curva, pelos quais a curva vai passar
// p.at(1) e p.at(2) são pontos de controle, que puxam a curva, mas não fazem parte dela.
void bezierCurve(std::vector<Point> p, Uint32 color) {
    double xu, yu, esp;

    for (double u = 0; u < 1; u += 0.0001) {
        esp = 1 - u;
        xu = ( pow(esp, 3) * p.at(0).x ) + ( 3*u * esp*esp * p.at(1).x ) + ( 3*esp * u*u * p.at(2).x ) + ( pow(u, 3) * p.at(3).x );
        yu = ( pow(esp, 3) * p.at(0).y ) + ( 3*u * esp*esp * p.at(1).y ) + ( 3*esp * u*u * p.at(2).y ) + ( pow(u, 3) * p.at(3).y );
        setPixel((int) xu, (int) yu, color);
    }
}


/**************************************** FUNÇÕES DE DESENHO ****************************************/


// Limpa a tela de volta para as constantes definidas em VERMELHO, VERDE e AZUL.
void resetScreen() {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            setPixel(x, y, RGB(VERMELHO,VERDE,AZUL));
        }
    }
}

// Vou usar isso só enquanto não tiver como escolher uma cor.
Uint32 getRandomColor() {
    int r, g, b;
    r = rand() % 255;
    g = rand() % 255;
    b = rand() % 255;

    return RGB(r, g, b);
}

// Deixa o programa em IDLE, aguardando o próximo comando por parte do usuário.
void setStatusIDLE() {
    points.clear();
    f = Function::None;
    printf("%sAguardando comando...\n", RUNTIME_LOG.c_str());
}

// Recebe o ponto em que o usuário clicou e trata de acordo com o tipo de operação.
void handleClick(Point p) {
    switch (f) {
        case Function::Line:
            points.push_back(p); 
            if (points.size() == 2) {
                bresenhamLine(points.at(0), points.at(1), getRandomColor());
                setStatusIDLE();
            }
            break;

        case Function::Rectangle:
            points.push_back(p);
            if (points.size() == 2) {
                rectangle(points.at(0), points.at(1), getRandomColor());
                setStatusIDLE();
            }
            break;

        case Function::Polygon:
            if (points.empty()) {
                printf("%sPonto inicial do Poligono (X, Y): (%d, %d).\n", FUNCTION_ARGS_LOG.c_str(), p.x, p.y);
            } else {
                bresenhamLine(points.back(), p, getRandomColor());
            }
            points.push_back(p);
            break;

        case Function::Circle:
            points.push_back(p);
            if (points.size() == 2) {
                int radius = euclideanDistance(points.at(0), points.at(1));
                bresenhamCircle(points.at(0), radius, getRandomColor());
                setStatusIDLE();
            }
            break;

        case Function::Bezier:
            points.push_back(p); 
            if (points.size() == 4) {
                bezierCurve(points, getRandomColor());
                setStatusIDLE();
            }
            break;

        case Function::Bucket: 
            floodFill(p, getRandomColor());
            setStatusIDLE();
            break;
        
        default:
            break;
    }

}

// Exporta o conteúdo da SDL_SURFACE para arquivo BMP.
void saveBMP() {
    std::string fileName = FILENAME_OUT + '_' + std::to_string(fileSeq++) + ".bmp";
    int result = SDL_SaveBMP( window_surface, fileName.c_str() );

    if ( result < 0 ) {
        printf("%sOcorreu um erro ao tentar salvar o arquivo.\n", ERROR_LOG.c_str());
    } else {
        printf("%sArquivo salvo com sucesso! Nome do arquivo: '%s'.\n", ROOT_LOG.c_str(), fileName.c_str());
    }
}

// Só conecta o último ponto do vetor de pontos ao primeiro quando a operação de desenhar Polígonos é encerrada.
inline void closePolygon() {
    bresenhamLine(points.back(), points.front(), getRandomColor());
}

// Monta a barra com todas as opções de desenho.
void displayToolbar() {
    int barHeight = 78;
    int barTop = height - barHeight;
    Point p1 = getPoint(0, barTop);
    Point p2 = getPoint(799, barTop);
    Uint32 barColor = RGB(27, 26, 35);

    bresenhamLine(p1, p2, barColor);
    floodFill(1, barTop + 1, barColor);

    Uint32 iconColor = RGB(234, 236, 234);
    int margin, x1, x2, y1, y2, iconSize;
    iconSize = 52;
    margin = (barHeight - iconSize) / 2;

    y1 = barTop + margin;
    y2 = y1 + iconSize;

    // Primeiros 8 botões
    for (int i = 0; i < 8; i++) {
        x1 = margin + (i * iconSize) + (i * margin);
        x2 = x1 + iconSize;
        rectangle(getPoint(x1, y1), getPoint(x2, y2), iconColor, true);
    }

    // Paleta de cores
}


int main(int argc, char* argv[]) {
    int x, y;

    setlocale(LC_ALL, NULL);
    srand(time(NULL));

    SDL_Init(SDL_INIT_VIDEO);

    if (argc == 2) {
        FILENAME_IN = argv[1];
        FILENAME_OUT = FILENAME_IN.substr(0, FILENAME_IN.find(".bmp"));
    }

    image = SDL_LoadBMP(FILENAME_IN.c_str());
    
    //TODO: CARREGAR PAINEL DE CONTROLE (IMG) AQUI!

        SDL_Window * window = SDL_CreateWindow(titulo.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            800, 600, SDL_WINDOW_SHOWN
        );

        window_surface = SDL_GetWindowSurface(window);
        if (image) {
            SDL_BlitSurface( image, NULL, window_surface, NULL );
        }

        pixels = (unsigned int *) window_surface->pixels;
        width = window_surface->w;
        height = window_surface->h;

        printf("Pixel format: %s\n", SDL_GetPixelFormatName(window_surface->format->format));
        bool firstRun = true;

        while (true) {
            if (firstRun && !image) {
                resetScreen();
                firstRun = false;
            }

        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }

            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    window_surface = SDL_GetWindowSurface(window);
                    pixels = (unsigned int *) window_surface->pixels;
                    width = window_surface->w;
                    height = window_surface->h;
                    printf("Size changed: %d, %d\n", width, height);
                }
            }

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_LCTRL:
                    case SDLK_RCTRL:
                        ctrlState = true;
                        break;

                    // CHAMADA --> FUNÇÕES DESENHO

                    case SDLK_l:
                        if (f != Function::Line) {
                            printf("%sDesenhar Linha.\n", FUNCTION_LOG.c_str());
                            f = Function::Line;
                        }
                        
                        break;

                    case SDLK_r:
                        if (f != Function::Rectangle) {
                            printf("%sDesenhar Retangulo.\n", FUNCTION_LOG.c_str());
                            f = Function::Rectangle;
                        }

                        break;

                    case SDLK_p:
                        if (f != Function::Polygon) {
                            printf("%sDesenhar Poligono.\n", FUNCTION_LOG.c_str());
                            f = Function::Polygon;
                        }

                        break;

                    case SDLK_c:
                        if (f != Function::Circle) {
                            printf("%sDesenhar Circulo.\n", FUNCTION_LOG.c_str());
                            f = Function::Circle;
                        }

                        break;

                    case SDLK_b:
                        if (f != Function::Bezier) {
                            printf("%sDesenhar Curva de Bezier.\n", FUNCTION_LOG.c_str());
                            f = Function::Bezier;
                        }

                        break;

                    case SDLK_f:
                        if (f != Function::Bucket) {
                            printf("%sFlood-Fill.\n", FUNCTION_LOG.c_str());
                            f = Function::Bucket;
                        }

                        break;

                    // FIM DE CHAMADA --> FUNÇÕES DESENHO

                    // CHAMADA --> FUNÇÕES ROOT

                    case SDLK_ESCAPE:
                        if (f == Function::Polygon) {
                            closePolygon();
                            printf("%s Fechando polígono...\n", ROOT_LOG.c_str());
                            setStatusIDLE();
                        } else if (f != Function::None) {
                            printf("%s Cancelando operacao...\n", ROOT_LOG.c_str());
                            setStatusIDLE();
                        }

                        break;

                    case SDLK_n:
                        if (ctrlState) {
                            printf("%sLimpando tela...\n", ROOT_LOG.c_str());
                            resetScreen();

                            break;
                        }

                    case SDLK_s:
                        if (ctrlState) {
                            printf("%sSalvando imagem...\n", ROOT_LOG.c_str());
                            saveBMP();
                            break;
                        }

                    case SDLK_z:
                        if (ctrlState) {
                            printf("%sDesfazendo alteracao...\n", ROOT_LOG.c_str());
                            // TODO Fazer o UNDO

                            break;
                        }
                    
                    case SDLK_q:
                        if (ctrlState) {
                            printf("%sSaindo...\n", ROOT_LOG.c_str());
                            SDL_DestroyWindow(window);
                            exit(0);
                        }

                    // FIM DE CHAMADA --> FUNÇÕES ROOT

                }
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_LCTRL || event.key.keysym.sym == SDLK_RCTRL) {
                    ctrlState = false;
                }
            }

            if (event.type == SDL_MOUSEMOTION) {
                showMousePosition(window,event.motion.x,event.motion.y);
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                x = event.motion.x;
                y = event.motion.y;
                
                /*Se o botão esquerdo do mouse é pressionado. */
                if(event.button.button == SDL_BUTTON_LEFT) {
                    handleClick(getPoint(x, y));
                }
            }
        }

        // displayToolbar(); // FIXME Algo aqui tá entrando em loop sem o resetScreen.

        SDL_UpdateWindowSurface(window);
    }
}

#define SDL_MAIN_HANDLED
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <stack>
#include <sstream>
#include <unistd.h>

using namespace std;

std::string FILENAME ="marvel-wallpaper.bmp";
std::string SAIDA ="saida.bmp";

// Estrutura para representar pontos
typedef struct {int x, y;} Point;

// variáveis necessárias para o SDL
unsigned int * pixels;
int width, height;
SDL_Surface * window_surface;
SDL_Surface * imagem;
SDL_Renderer * renderer;

// Título da janela
std::string titulo = "SDL BMP ";

// Valores RGB para a cor de fundo da janela
const int VERMELHO = 0;
const int VERDE = 0;
const int AZUL = 0;

// Gera uma estrutura Point a partir de valores para x e y
Point getPoint(int x, int y)
{
    Point p;
    p.x = x;
    p.y = y;
    return p;
}

// Obtém a cor de um pixel de uma determinada posição
Uint32 getPixel(int x, int y)
{
    if((x>=0 && x<=width) && (y>=0 && y<=height))
        return pixels[x + width * y];
    else
        return -1;
}

// seta um pixel em uma determinada posição,
// através da coordenadas de cor r, g, b, e alpha (transparência)
// r, g, b e a variam de 0 até 255
void setPixel(int x, int y, int r, int g, int b, int a)
{
    pixels[x + y * width] = SDL_MapRGBA(window_surface->format, r, g, b, a);
}

// seta um pixel em uma determinada posição,
// através da coordenadas de cor r, g e b
// r, g, e b variam de 0 até 255
void setPixel(int x, int y, int r, int g, int b)
{
    setPixel(x, y, r, g, b, 255);
}

// Mostra na barra de título da janela a posição
// corrente do mouse
void showMousePosition(SDL_Window * window, int x, int y)
{
    std::stringstream ss;
    ss << titulo << " X: " << x << " Y: " << y;
    SDL_SetWindowTitle(window, ss.str().c_str());
}

// Imprime na console a posição corrente do mouse
void printMousePosition(int x, int y)
{
    printf("Mouse on x = %d, y = %d\n",x,y);
}

// seta um pixel em uma determinada posição,
// através de um Uint32 representando
// uma cor RGB
void setPixel(int x, int y, Uint32 color)
{
    if((x>=0 && x<width && y>=0 && y<height)) {
        pixels[x + y * width] = color;
    }
}

// retorna uma cor RGB(UInt32) formada
// pelas componentes r, g, b e a(transparência)
// informadas. r, g, b e a variam de 0 até 255
Uint32 RGB(int r, int g, int b, int a) {
    return SDL_MapRGBA(window_surface->format, r, g, b, a);
}

// retorna uma cor RGB(UInt32) formada
// pelas componentes r, g, e b
// informadas. r, g e b variam de 0 até 255
// a transparência é sempre 255 (imagem opaca)
Uint32 RGB(int r, int g, int b) {
    return SDL_MapRGBA(window_surface->format, r, g, b, 255);
}

// retorna um componente de cor de uma cor RGB informada
// aceita os parâmetros 'r', 'R','g', 'G','b' e 'B',
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

void bresenham(int x1, int y1, int x2, int y2, Uint32 cor)
{
    int x,y,dx,dy,dx1,dy1,px,py,xe,ye,i;

    dx=x2-x1;
    dy=y2-y1;

    dx1=fabs(dx);
    dy1=fabs(dy);

    px=2*dy1-dx1;
    py=2*dx1-dy1;

    if(dy1<=dx1)
    {
        if(dx>=0)
        {
            x=x1;
            y=y1;
            xe=x2;
        }
        else
        {
            x=x2;
            y=y2;
            xe=x1;
        }
        setPixel(x,y,cor);
        for(i=0;x<xe;i++)
        {
            x=x+1;
            if(px<0)
            {
                px=px+2*dy1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    y=y+1;
                }
                else
                {
                    y=y-1;
                }
                px=px+2*(dy1-dx1);
            }
            setPixel(x,y,cor);
        }
    }
    else
    {
        if(dy>=0)
        {
            x=x1;
            y=y1;
            ye=y2;
        }
        else
        {
            x=x2;
            y=y2;
            ye=y1;
        }
        setPixel(x,y,cor);
        for(i=0;y<ye;i++)
        {
            y=y+1;
            if(py<=0)
            {
                py=py+2*dx1;
            }
            else
            {
                if((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    x=x+1;
                }
                else
                {
                    x=x-1;
                }
                py=py+2*(dx1-dy1);
            }
            setPixel(x,y,cor);
        }
    }
}

void quadriculado(int distancia, Uint32 cor)
{
    for (int x = 0; x < width; x++)
    {
        if(x % distancia == 0) {
            bresenham(x,0,x,height-1,cor);
        }
    }

    for (int y = 0; y < height; y++)
    {
        if(y % distancia == 0) {
            bresenham(0,y,width-1,y,cor);
        }
    }
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

Uint32 getRandomColor() {

    int r, g, b;
    r = rand() % 255;
    g = rand() % 255;
    b = rand() % 255;

    return RGB(r, g, b);
}

void displayBresenhamCircle(int xc, int yc, int x, int y, Uint32 color) {
    // Desenha todas as 8 coordenadas de (x, y), uma para cada octante

    setPixel(xc+x, yc+y, color);
    setPixel(xc-x, yc+y, color);
    setPixel(xc+x, yc-y, color);
    setPixel(xc-x, yc-y, color);
    setPixel(xc+y, yc+x, color);
    setPixel(xc-y, yc+x, color);
    setPixel(xc+y, yc-x, color);
    setPixel(xc-y, yc-x, color);

}

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

void bresenhamCircle(Point p, int radius, Uint32 color) {
    int xc = p.x;
    int yc = p.y;
    bresenhamCircle(xc, yc, radius, color);
}

void floodFill(int x, int y, Uint32 newColor, Uint32 oldColor) {

    if (x < 0 || x > width - 1 || y < 0 || y > height - 1) {
        return;
    }

    stack<Point> st;
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

void floodFill(Point p, Uint32 newColor, Uint32 oldColor) {
    int x, y;
    x = p.x;
    y = p.y;
    floodFill(x, y, newColor, oldColor);
}

void bezierCurve(Point p[4], Uint32 color) {
    double xu, yu, esp;

    for (double u = 0; u < 1; u += 0.0001) {
        esp = 1 - u;
        xu = ( pow(esp, 3) * p[0].x ) + ( 3*u * esp*esp * p[1].x ) + ( 3*esp * u*u * p[2].x ) + ( pow(u, 3) * p[3].x );
        yu = ( pow(esp, 3) * p[0].y ) + ( 3*u * esp*esp * p[1].y ) + ( 3*esp * u*u * p[2].y ) + ( pow(u, 3) * p[3].y );
        setPixel((int) xu, (int) yu, color);
    }
}

void rectangle(Point p1, Point p2, Uint32 color, bool fill) {
    bresenham(p1.x, p1.y, p2.x, p1.y, color);
    bresenham(p1.x, p1.y, p1.x, p2.y, color);
    bresenham(p1.x, p2.y, p2.x, p2.y, color);
    bresenham(p2.x, p1.y, p2.x, p2.y, color);

    if (fill) {
        for (int y = p1.y; y < p2.y; y++) {
            bresenham(p1.x, y, p2.x, y, color);
        }
    }
}

// Aqui ocorrem as chamadas das funções a ser exibidas na janela
void display()
{
    Point p1, p2;
    p1 = getPoint(0, 599 - 78);
    p2 = getPoint(799, 599);

    Uint32 color = RGB(210, 107, 70);

    rectangle(p1, p2, color, true);
}

void reset_screen()
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            setPixel(x, y, RGB(VERMELHO,VERDE,AZUL));
        }
    }
}

//void copia_para_bmp(Bit)
// Inicializa o SDL, abre a janela e controla o loop
// principal do controle de eventos
int main()
{
    // Inicializações iniciais obrigatórias

    int result;

    setlocale(LC_ALL, NULL);

    SDL_Init(SDL_INIT_VIDEO);

    imagem = SDL_LoadBMP( FILENAME.c_str() );

    if(imagem) {

        SDL_Window * window = SDL_CreateWindow(titulo.c_str(),
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            imagem->w, imagem->h,
            SDL_WINDOW_RESIZABLE);

        window_surface = SDL_GetWindowSurface(window);

        SDL_BlitSurface( imagem, NULL, window_surface, NULL );

        pixels = (unsigned int *) window_surface->pixels;
        width = window_surface->w;
        height = window_surface->h;

        // Fim das inicializações

        printf("SDL Pixel format: %s\n",
            SDL_GetPixelFormatName(window_surface->format->format));

        while (1)
        {

            SDL_Event event;

            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    exit(0);
                }

                if (event.type == SDL_KEYDOWN) {
                    switch(event.key.keysym.sym) {
                        case SDLK_s:

                            result = SDL_SaveBMP( window_surface, SAIDA.c_str() );
                            if ( result < 0 ) {
                                printf("Ocorreu um erro salvando o arquivo\n");
                            } else {
                                printf("Arquivo salvo com sucesso!\n");
                            }
                            //saveImg(SAIDA,bmp);
                            break;

                        case SDLK_q:

                            quadriculado(20, RGB(255, 0, 0));
                            break;
                        }
                }

                if (event.type == SDL_WINDOWEVENT)
                {
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        window_surface = SDL_GetWindowSurface(window);
                        pixels = (unsigned int *) window_surface->pixels;
                        width = window_surface->w;
                        height = window_surface->h;
                        printf("Size changed: %d, %d\n", width, height);
                        SDL_BlitSurface( imagem, NULL, window_surface, NULL );
                    }
                }

                // Se o mouse é movimentado
                if(event.type == SDL_MOUSEMOTION)
                {
                    // Mostra as posições x e y do mouse
                    showMousePosition(window,event.motion.x,event.motion.y);
                }
                if(event.type == SDL_MOUSEBUTTONDOWN)
                {
                    /*Se o botão esquerdo do mouse é pressionado */
                    if(event.button.button == SDL_BUTTON_LEFT)
                    {
                        printf("Mouse pressed on (%d,%d)\n",event.motion.x,event.motion.y) ;
                    }
                }
            }

            // Seta a cor de fundo da janela para a informada nas
            // constantes VERMELHO, VERDE e AZUL
            //reset_screen();

            //mostra_bmp(bmp);

            display();

            SDL_UpdateWindowSurface(window);
        }

    }
}

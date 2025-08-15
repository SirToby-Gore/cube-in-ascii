import 'dart:io';
import 'dart:math';

String getCharRed(String chr) {
  return "\x1B[31m${chr}\x1B[0m";
}

String getCharBlue(String chr) {
  return "\x1B[34m${chr}\x1B[0m";
}

class Vector {
  double x;
  double y;
  double z;

  Vector(this.x, this.y, this.z);
}

const WIDTH = 20;
const HEIGHT = 20;

final String character1 = getCharRed(' ');
final String character2 = getCharBlue('@');
final double scale = min(WIDTH, HEIGHT) / 2;
final List<List<String?>> pixels = List.generate(WIDTH, (_) => List.generate(HEIGHT, (_) => ''));
const double cameraDistance = 100;
const double offsetX = 0;
const double offsetY = 0;
const double offsetZ = 0;
double cameraAngleX = 0;
double cameraAngleY = 0;
double cameraAngleZ = 0;


void drawLine(int x0, int y0, int x1, int y1) {
  bool steep = ((y1 - y0).abs() > (x1 - x0).abs());

  if (steep) {
    int t = x0;
    x0 = y0;
    y0 = t;
    t = x1;
    x1 = y1;
    y1 = t;
  }

  if (x0 > x1) {
    int t = x0;
    x0 = x1;
    x1 = t;
    t = y0;
    y0 = y1;  y1 = t;
  }

  int dx = x1 - x0;
  int dy = (y1 - y0).abs();
  int err = (dx / 2).round();
  int ystep = (y0 < y1) ? 1 : -1;
  int y = y0;

  for (int x = x0; x <= x1; x++) {
    if (steep) {
      if (y >= 0 && y < WIDTH && x >= 0 && x < HEIGHT) {
        pixels[y][x] = character2;
      }
    } else {
      if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        pixels[x][y] = character2;
      }
    }
    err -= dy;
    if (err < 0) {
      y += ystep;
      err += dx;
    }
  }
}


void draw() {
  
  List<Vector> vertices = [
    Vector(offsetX - 0.5, offsetY - 0.5, offsetZ - 0.5),
    Vector(offsetX - 0.5, offsetY - 0.5, offsetZ + 0.5),
    Vector(offsetX - 0.5, offsetY + 0.5, offsetZ - 0.5),
    Vector(offsetX - 0.5, offsetY + 0.5, offsetZ + 0.5),
    Vector(offsetX + 0.5, offsetY - 0.5, offsetZ - 0.5),
    Vector(offsetX + 0.5, offsetY - 0.5, offsetZ + 0.5),
    Vector(offsetX + 0.5, offsetY + 0.5, offsetZ - 0.5),
    Vector(offsetX + 0.5, offsetY + 0.5, offsetZ + 0.5),
  ];
  
  int numElements = vertices.length;

  List<Vector> projectedVertices = List.generate(8, (_) => Vector(0, 0, 0));

  double offsetCanvasX = WIDTH / 2;
  double offsetCanvasY = HEIGHT / 2;

  Vector(1,1,1);
  int i = 0;
  
  while (i < numElements) {
    double x = vertices[i].x;
    double y = vertices[i].y;
    double z = vertices[i].z;


    double tmpY = y * cos(cameraAngleX) - z * sin(cameraAngleX);
    double tmpZ = y * sin(cameraAngleX) + z * cos(cameraAngleX);

    double tmpX = x * cos(cameraAngleY) + tmpZ * sin(cameraAngleY);
    tmpZ = -x * sin(cameraAngleY) + tmpZ * cos(cameraAngleY);

    x = tmpX * cos(cameraAngleZ) - tmpY * sin(cameraAngleZ);
    y = tmpX * sin(cameraAngleZ) + tmpY * cos(cameraAngleZ);

    double perspective = cameraDistance / (tmpZ + cameraDistance);
    projectedVertices[i].x = x * perspective * scale + offsetCanvasX;
    projectedVertices[i].y = y * perspective * scale + offsetCanvasY;
    
    i++;
  }
  
  List<List<int>> edges = [
    [0, 1], [1, 3], [3, 2], [2, 0],
    [4, 5], [5, 7], [7, 6], [6, 4],
    [0, 4], [1, 5], [2, 6], [3, 7]
  ];

  int marker = 0;
  numElements = edges.length;
  
  while (marker < numElements) {
    int x1 = projectedVertices[edges[marker][0]].x.toInt();
    int y1 = projectedVertices[edges[marker][0]].y.toInt();
    int x2 = projectedVertices[edges[marker][1]].x.toInt();
    int y2 = projectedVertices[edges[marker][1]].y.toInt();
    drawLine(x1, y1, x2, y2);    
    marker++;
  }
}

void resetCanvas() {
  for (int x = 0; x < HEIGHT - 1; x++) {
    for (int y = 0; y < WIDTH - 1; y++) {
      pixels[y][x] = character1;
    }
  }
}

void refresh() {
  stdout.write("\x1B[2J\x1B[0;0H"); 
}

void printPixels() {
  print(
    pixels.map(
      (row) => row.join('')
    ).join('\n')
  );
}

void initScreen() {
  refresh();
  stdout.write('\x1B[0m');
}

void main() {
  initScreen();

  while (1 > 0) {
    resetCanvas();
    cameraAngleX+=0.03; 
    cameraAngleY+=0.03;
    draw();
    printPixels();
    sleep(Duration(milliseconds: 1));
    refresh();
  }
}

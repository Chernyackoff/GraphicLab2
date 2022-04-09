#include <QApplication>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLayout>
#include <QGraphicsItem>
#include <QLabel>
#include <QPoint>
#include <QKeyEvent>
#include <cmath>
#include <vector>

const int delta_move = 10;
const double rotation_angle = 15 * std::numbers::pi / 180;
const std::array<QPointF, 6> default_points = {QPointF(10, 10),
                                               {30, 10},
                                               {35, 20},
                                               {30, 30},
                                               {10, 30},
                                               {5, 20}
};

class Hexagon {
public:
    Hexagon(QGraphicsScene *scene_, const std::array<QPointF, 6> &points_) : scene(scene_), points(points_), lines(),
                                                                             centre(), paintLines(){
        double cx = 0, cy = 0;
//        QString letter = "ABCDEF";
        for (int i = 0; i < 6; i++) {
            lines[i] = scene->addLine({points[i], points[(i + 1) % 6]});
            cx += points[i].x();
            cy += points[i].y();

//            letters[i] = scene->addText(letter[i]);
//            letters[i]->setPos(points[i]);
//            letters[i]->setParentItem(lines[i]);
        }
        centre.setX(cx / 6);
        centre.setY(cx / 6);
        paint();
    }

    ~Hexagon() {
        for (auto &l: lines) {
            scene->removeItem(l);
            delete l;
        }
    }

    void move(float dx, float dy) {
        for (auto &point: points) {
            point.rx() += dx;
            point.ry() += dy;
        }
        centre.rx() += dx;
        centre.ry() += dy;
        update();
    }

    void rotate(double alpha) {
        for (auto &point: points) {
            auto dx = point.x() - centre.x(), dy = point.y() - centre.y();
            double sx, sy;
            sx = dx * cos(alpha) - dy * sin(alpha);
            sy = dx * sin(alpha) + dy * cos(alpha);
            point.setX(sx + centre.x());
            point.setY(sy + centre.y());
        }
        update();
    };

    void resize(double delta) {
        for (auto &point: points) {
            auto dx = point.x() - centre.x(), dy = point.y() - centre.y();
            double sx, sy;
            sx = dx * delta;
            sy = dy * delta;
            point.setX(sx + centre.x());
            point.setY(sy + centre.y());
        }
        update();
    };

private:
    QGraphicsScene *scene;
    std::array<QPointF, 6> points;
    std::array<QGraphicsLineItem *, 6> lines;
    std::vector<QGraphicsLineItem *> paintLines;
    QPointF centre;
//    std::array<QGraphicsTextItem *,6> letters;

    void paint(){
        QPen pen;
        pen.setColor(Qt::green);
        pen.setStyle(Qt::SolidLine);
        // 1rst triangulation pattern
//        draw(points[0], points[1], points[4]);
//        draw(points[1], points[2], points[3]);
//        draw(points[1], points[3], points[4]);
//        draw(points[4], points[5], points[0]);
        // 2nd triangulation pattern
        for (int i = 1; i < lines.size() - 1; i++){
            draw(points[0], points[i], points[i+1], pen);
        }
    }
    void draw(QPointF p1, QPointF p2, QPointF p3, QPen pen){
        // sort in ascending order from p1 to p3
        if (p1.y() > p2.y()) std::swap(p1, p2);
        if (p2.y() > p3.y()) std::swap(p2, p3);
        if (p2.y() < p1.y()) std::swap(p2, p1);

        // ratios of line equation i.e (x - x1)/(x2 - x1) = (y - y1)/(y2 - y1)
        // kX = x2 - x1
        // ky = y2 - y1
        int kX1, kX2, kY1, kY2;
        int x0 = p1.x(), y0 = p1.y();

        bool switched = false;

        kX1 = p2.x() - p1.x();
        kY1 = p2.y() - p1.y();
        kX2 = p3.x() - p1.x();
        kY2 = p3.y() - p1.y();

        // intersection points
        qreal x1, x2;
        for (int y = p1.y() + 1; y <= p3.y(); y += 1) {
            if (y >= int(p2.y()) || kY1 == 0) {
                switched = true;
                kX1 = p3.x() - p2.x();
                kY1 = p3.y() - p2.y();
                x0 = p2.x();
                y0 = p2.y();
            }

            x1 = double ((y - y0) * kX1) / kY1 + x0;
            x2 = double ((y - p1.y()) * kX2) / kY2 + p1.x();

            if (kX1 == 0) x1 = switched ? p2.x() : p1.x();
            if (kX2 == 0) x2 = p1.x();

            paintLines.push_back(scene->addLine(
                    {QPointF(x1, y), QPointF(x2, y)}, pen));
        }
    }

    void update() {
        for(auto line : paintLines){
            scene->removeItem(line);
            delete line;
        }
        paintLines.clear();
        for (int i = 0; i < 6; i++) {
            lines[i]->setLine({points[i], points[(i + 1) % 6]});
//            letters[i]->setPos(points[i]);

        }
        paint();
    }
};

class MainWindow : public QMainWindow {
public:
    MainWindow() : QMainWindow(), scene(), view(&scene) {
        resize(800, 600);
        auto central = new QWidget(this);
        auto layout = new QHBoxLayout(central);
        central->setLayout(layout);
        setCentralWidget(central);

        auto label = new QLabel("Controls:\n1. Movement: WASD\n2. Rotation: QE\n3. Scale: ZC\n",
                                central);
        layout->addWidget(label);
        layout->addWidget(&view);

        scene.setParent(central);
        view.setParent(central);

        scene.setSceneRect(0, 0, 550, 550);
        view.setSceneRect(0, 0, 550, 550);

        hex = new Hexagon(&scene, default_points);
    }

    void keyPressEvent(QKeyEvent *event) override {
        switch (event->key()) {
            case Qt::Key_W:
                hex->move(0, -delta_move);
                break;
            case Qt::Key_S:
                hex->move(0, delta_move);
                break;
            case Qt::Key_A:
                hex->move(-delta_move, 0);
                break;
            case Qt::Key_D:
                hex->move(delta_move, 0);
                break;
            case Qt::Key_Q:
                hex->rotate(-rotation_angle);
                break;
            case Qt::Key_E:
                hex->rotate(rotation_angle);
                break;
            case Qt::Key_Z:
                hex->resize(0.7);
                break;
            case Qt::Key_C:
                hex->resize(1.3);
                break;
        }
    }

private:
    QGraphicsScene scene;
    QGraphicsView view;
    Hexagon *hex;
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow window;
    window.show();
    return QApplication::exec();
}

class Shape 
{
　public:    
　   virtual void draw() = 0;
};

class Rectangle : public Shape {
　public:    
　    void draw() override {        
　        std::cout << "Drawing a rectangle." << std::endl;    
　    }
};

class Circle : public Shape {
public: 
　 void draw() override {        
　         std::cout << "Drawing a circle." << std::endl;    
　     }
};

int main()
{    
　    using rectangleBridge = bridge<Shape, Rectangle>;
　    using circleBridge = bridge<Shape, Circle> ;

　    Shape* rectangle = rectangleBridge.create();    
　    Shape* circle = circleBridge.create();        
　    rectangle->draw();    
　    circle->draw();        

　    delete rectangle;    
　    delete circle;        
　    return 0;
}
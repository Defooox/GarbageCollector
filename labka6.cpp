#include <iostream>
#include <vector>
#include <unordered_set>

class GarbageCollector;

class GarbageCollectable {
public:
    virtual ~GarbageCollectable() = default;
    virtual void mark() = 0;
    virtual void sweep(GarbageCollector& collector) = 0;
};

class GarbageCollector {
private:
    std::unordered_set<GarbageCollectable*> allocatedObjects;
    std::vector<GarbageCollectable*> markedObjects;

public:
    void* allocate(size_t size) {
        void* memory = ::operator new(size);
        GarbageCollectable* object = static_cast<GarbageCollectable*>(memory);
        allocatedObjects.insert(object);
        return memory;
    }

    template<typename T, typename... Args>
    T* createObject(Args&&... args) {
        T* object = new (allocate(sizeof(T))) T(std::forward<Args>(args)...);
        return object;
    }

    void deallocate(GarbageCollectable* object) {
        auto it = allocatedObjects.find(object);
        if (it != allocatedObjects.end()) {
            allocatedObjects.erase(it);
            object->~GarbageCollectable();
            ::operator delete(object);
        }
    }

    void markObject(GarbageCollectable* object) {
        markedObjects.push_back(object);
    }

    void mark() {
        for (GarbageCollectable* object : allocatedObjects) {
            object->mark();
        }
    }

    void sweep() {
        for (auto it = allocatedObjects.begin(); it != allocatedObjects.end();) {
            GarbageCollectable* object = *it;
            if (std::find(markedObjects.begin(), markedObjects.end(), object) == markedObjects.end()) {
                it = allocatedObjects.erase(it);
                object->sweep(*this);
            }
            else {
                ++it;
            }
        }
        markedObjects.clear();
    }

    ~GarbageCollector() {
        for (GarbageCollectable* object : allocatedObjects) {
            object->~GarbageCollectable();
            ::operator delete(object);
        }
    }
};

class MyClass : public GarbageCollectable {
private:
    int data;

public:
    MyClass(int value) : data(value) {}

    void mark() override {
        // In a real application, we need to recursively label related objects
        std::cout << "Marking MyClass object" << std::endl;
    }

    void sweep(GarbageCollector& collector) override { 
        //Example of releasing object resources
        std::cout << "Sweeping MyClass object" << std::endl;
        collector.deallocate(this);
    }
};

int main() {
    GarbageCollector garbageCollector;

    MyClass* myObject = garbageCollector.createObject<MyClass>(42);

   
    garbageCollector.mark();
    garbageCollector.sweep();

    return 0;
}

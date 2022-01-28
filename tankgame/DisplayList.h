#ifndef DisplayList_h
#define DisplayList_h

class DisplayList {
public:
    DisplayList(int num = 0);
    void BeginNewList();
    void NextNewList();
    void EndNewList();
    void ResetList();
    void NewList();
    void EndList();
    void Call(int i);
    void Close();

private:
    int num;
    int idx;
    int enumIdx;
};

#endif /* DisplayList_h */

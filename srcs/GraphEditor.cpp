#include "GraphEditor.h"

GraphEditor& GraphEditor::instance(int argc, char *argv[])
{
    static GraphEditor app(argc, argv);
    return app;
}

GraphEditor::GraphEditor(int argc, char *argv[])
{

}

#include "Output.h"

#include "UserOutput.h"

void endOfLoad(Html *parser)
{
    loaded(parser);
}

void initOutput()
{
    initUserOutput();
}
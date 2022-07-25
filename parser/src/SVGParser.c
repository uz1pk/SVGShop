#include "SVGParser.h"
#include "SVGHelper.h"

/*
Main SVG Parsing Functions Below
*/

SVG *createSVG(const char *filename)
{
    if (!filename)
    {
        fprintf(stderr, "Cannnot give a NULL file name\n");
        return NULL;
    }
    else if (strlen(filename) <= 0)
    {
        fprintf(stderr, "Cannnot give an empty file name\n");
        return NULL;
    }

    SVG *finishedSVG = (SVG *)malloc(sizeof(SVG));
    xmlDoc *doc = NULL;
    xmlNode *rootElement = NULL;

    if (!finishedSVG)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    LIBXML_TEST_VERSION

    doc = xmlReadFile(filename, NULL, 0);
    if (!doc)
    {
        fprintf(stderr, "Could not open XML given XML file\n");
        return NULL;
    }

    // Check and get nameSpace
    rootElement = xmlDocGetRootElement(doc);
    if (rootElement->ns->href != NULL)
    {
        if (strlen((char *)rootElement->ns->href) >= 256)
        {
            strncpy(finishedSVG->namespace, (char *)rootElement->ns->href, 255);
            finishedSVG->namespace[255] = '\0';
        }
        else if (strlen((char *)rootElement->ns->href) > 0)
        {
            strcpy(finishedSVG->namespace, (char *)rootElement->ns->href);
        }
    }

    /*
    Initialize all SVG elements/properties, remeber anything can be empty,
    but NOTHING can be NULL.
    */
    finishedSVG->title[0] = '\0';
    finishedSVG->description[0] = '\0';
    finishedSVG->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
    finishedSVG->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
    finishedSVG->paths = initializeList(&pathToString, &deletePath, &comparePaths);
    finishedSVG->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
    if (!(finishedSVG->groups) || !(finishedSVG->paths) || !(finishedSVG->circles) || !(finishedSVG->rectangles))
    {
        fprintf(stderr, "SVG List initializations failed\n");
        return NULL;
    }

    /*
    The parseTree function will traverse the xml document using depth first
    traversal, from there it will extract all data and start populating the SVG
    struct with all corresponding information within the document.
    If parsing fails at any point, -1 will be returned and any allocated memory prior
    to parseing failure will be freed and function will return NULL
    */

    if (parseTree(finishedSVG, rootElement) != 0)
    {
        fprintf(stderr, "Parsing failed\n");
        deleteSVG(finishedSVG);
        return NULL;
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();

    return finishedSVG;
}

void deleteSVG(SVG *img)
{
    if (!img)
    {
        fprintf(stderr, "SVG cannot be NULL\n");
        return;
    }
    else if (!(img->rectangles) || !(img->circles) || !(img->paths) || !(img->groups) || !(img->otherAttributes))
    {
        fprintf(stderr, "SVG Lists cannot be NULL\n");
        return;
    }
    freeList(img->rectangles);
    freeList(img->circles);
    freeList(img->paths);
    freeList(img->groups);
    freeList(img->otherAttributes);
    free(img);
}

char *SVGToString(const SVG *img)
{
    if (!img)
    {
        fprintf(stderr, "SVG cannot be NULL\n");
        return NULL;
    }
    else if (!(img->rectangles) || !(img->circles) || !(img->paths) || !(img->groups) || !(img->otherAttributes))
    {
        fprintf(stderr, "SVG Lists cannot be NULL\n");
        return NULL;
    }

    /*
    Get the strings representing all data withing all the Lists
    in the SVG structure
    - rectangles, circles, paths, groups and other attributes
    */
    char *rectString = toString(img->rectangles);
    if (!rectString)
    {
        fprintf(stderr, "SVGtoString Rectanlges string toString failed\n");
        return NULL;
    }

    char *circleString = toString(img->circles);
    if (!circleString)
    {
        fprintf(stderr, "SVGtoString Circles string toString failed\n");
        return NULL;
    }

    char *pathString = toString(img->paths);
    if (!pathString)
    {
        fprintf(stderr, "SVGtoString Paths string toString failed\n");
        return NULL;
    }

    char *groupString = toString(img->groups);
    if (!groupString)
    {
        fprintf(stderr, "SVGtoString Groups string toString failed\n");
        return NULL;
    }

    char *attrString = toString(img->otherAttributes);
    if (!attrString)
    {
        fprintf(stderr, "SVGtoString other attributes toString failed\n");
        return NULL;
    }

    // Calculate all memory needed for the SVG string representation and allocate
    int memLen = 0;
    memLen = strlen(img->namespace) + strlen(attrString) + strlen(img->title) + strlen(img->description) + strlen(rectString) + strlen(circleString) + strlen(pathString) + strlen(groupString) + 78;

    char *stringToReturn = (char *)malloc(sizeof(char) * memLen);
    if (!stringToReturn)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    // This will copy the strings into the return string with the given data format.
    sprintf(stringToReturn, "SVG Attributes:\n\tNamespace: %s%s\nSVG Title:\n\t%s\nSVG Description:\n\t%s\n%s%s%s%s", img->namespace, attrString, img->title, img->description, rectString, circleString, pathString, groupString);

    // free all strings returned by toString and return the final String.

    free(rectString);
    free(circleString);
    free(pathString);
    free(groupString);
    free(attrString);

    return stringToReturn;
}

/*
All Attribute Helpers Below
*/

void deleteAttribute(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot delete a NULL Attribute\n");
        return;
    }

    Attribute *attr = (Attribute *)data;
    if (!(attr->name) && (attr->value))
    {
        free(attr);
    }
    else
    {
        free(attr->name);
        free(attr);
    }
}

char *attributeToString(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot use NULL Attribute to create String\n");
        return NULL;
    }

    Attribute *attr = (Attribute *)data;

    if (!(attr->name) || !(attr->value))
    {
        fprintf(stderr, "Cannot use NULL Attribute elements to create String\n");
        return NULL;
    }
    int memLen = strlen(attr->name) + strlen(attr->value) + 37;
    char *returnString = (char *)malloc(sizeof(char) * memLen);
    if (!returnString)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    strcpy(returnString, "\tAttribute Name: ");
    strcat(returnString, attr->name);
    strcat(returnString, ", Attribute Value: ");
    strcat(returnString, attr->value);

    return returnString;
}

int compareAttributes(const void *first, const void *second)
{
    if (!first || !second)
    {
        fprintf(stderr, "Cannot compare NULL attributes compareAttributes\n");
        return -1;
    }

    Attribute *attr1 = (Attribute *)first;
    Attribute *attr2 = (Attribute *)second;

    if (!(attr1->name) || !(attr2->name))
    {
        fprintf(stderr, "Cannot compare NULL attributes names compareAttributes\n");
        return -1;
    }

    if (strcasecmp(attr1->name, attr2->name) == 0)
    {
        return 0;
    }
    return 1;
}

/*
All Group Helpers Below
*/

void deleteGroup(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot delete a NULL Group\n");
        return;
    }

    Group *group = (Group *)data;
    if (!(group->rectangles) || !(group->circles) || !(group->paths) || !(group->groups) || !(group->otherAttributes))
    {
        fprintf(stderr, "Cannot free uninitialized lists\n");
        return;
    }

    freeList(group->rectangles);
    freeList(group->circles);
    freeList(group->paths);
    freeList(group->groups);
    freeList(group->otherAttributes);
    free(group);
}

char *groupToString(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot use NULL Group to create String\n");
        return NULL;
    }

    Group *img = (Group *)data;

    if (!(img->rectangles) || !(img->circles) || !(img->paths) || !(img->groups))
    {
        fprintf(stderr, "Cannot use NULL Group lists to create String\n");
        return NULL;
    }

    char *rectString = toString(img->rectangles);
    if (!rectString)
    {
        fprintf(stderr, "groupToString: Rectanlges toString failed\n");
        return NULL;
    }

    char *circleString = toString(img->circles);
    if (!circleString)
    {
        fprintf(stderr, "groupToString: Circles toString failed\n");
        return NULL;
    }

    char *pathString = toString(img->paths);
    if (!pathString)
    {
        fprintf(stderr, "groupToString: Paths toString failed\n");
        return NULL;
    }

    char *groupString = toString(img->groups);
    if (!groupString)
    {
        fprintf(stderr, "groupToString: Groups toString failed\n");
        return NULL;
    }

    char *attrString = toString(img->otherAttributes);
    if (!attrString)
    {
        fprintf(stderr, "groupToString: Other Attributes toString failed\n");
        return NULL;
    }

    int memLen = 0;
    memLen += strlen(rectString);
    memLen += strlen(circleString);
    memLen += strlen(pathString);
    memLen += strlen(groupString);
    memLen += strlen(attrString) + 100;

    char *stringToReturn = (char *)malloc(sizeof(char) * memLen);
    if (!stringToReturn)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    sprintf(stringToReturn, "Group:\n\tGroup Attributes:\n%s\n\nGroup Elements: {%s\n%s\n%s\n%s} End of Group\n", attrString, rectString, circleString, pathString, groupString);

    free(rectString);
    free(circleString);
    free(pathString);
    free(groupString);
    free(attrString);

    return stringToReturn;
}

int compareGroups(const void *first, const void *second)
{
    return 0;
}

/*
All Rectangle Helpers Below
*/

void deleteRectangle(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot delete a NULL Rectangle\n");
        return;
    }

    Rectangle *rect = (Rectangle *)data;
    freeList(rect->otherAttributes);
    free(rect);
}

char *rectangleToString(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot use NULL Rectangle for toString\n");
        return NULL;
    }

    Rectangle *rect = (Rectangle *)data;
    if (!(rect->otherAttributes))
    {
        fprintf(stderr, "Cannot use NULL Rectangle other attributes for toString\n");
        return NULL;
    }

    char *otherAttributes = toString(rect->otherAttributes);
    if (!otherAttributes)
    {
        fprintf(stderr, "Reactangle to string failed creating attributes to string\n");
        return NULL;
    }

    int memLen = strlen(otherAttributes) + 200;
    char *returnString = (char *)malloc(sizeof(char) * memLen);
    if (!returnString)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    if (rect->units[0] == '0')
    {
        sprintf(returnString, "Rectangle Data:\n\tX-Coordinate: %f\n\tY-Coordinate: %f\n\tWidth: %f\n\tHeight: %f\n", rect->x, rect->y, rect->width, rect->height);
        strcat(returnString, otherAttributes);
    }
    else
    {
        sprintf(returnString, "Rectangle Data:\n\tX-Coordinate: %f\n\tY-Coordinate: %f\n\tWidth: %f\n\tHeight: %f\n\tUnits: %s\n", rect->x, rect->y, rect->width, rect->height, rect->units);
        strcat(returnString, otherAttributes);
    }

    free(otherAttributes);

    return returnString;
}

int compareRectangles(const void *first, const void *second)
{
    return 0;
}

/*
All Circle Helpers Below
*/

void deleteCircle(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot delete a NULL Circle\n");
        return;
    }

    Circle *circle = (Circle *)data;
    freeList(circle->otherAttributes);
    free(circle);
}

char *circleToString(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot use NULL Circle for toString\n");
        return NULL;
    }

    Circle *circle = (Circle *)data;
    if (!(circle->otherAttributes))
    {
        fprintf(stderr, "Cannot use NULL Circle other attributes for toString\n");
        return NULL;
    }
    char *otherAttributes = toString(circle->otherAttributes);
    if (!otherAttributes)
    {
        fprintf(stderr, "Circle to string failed creating attributes to string\n");
        return NULL;
    }

    int memLen = strlen(otherAttributes) + 147;
    char *returnString = (char *)malloc(sizeof(char) * memLen);
    if (!returnString)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    if (circle->units[0] == '0')
    {
        sprintf(returnString, "Circle Data:\n\tCenter X-Coordinate: %f\n\tCenter Y-Coordinate: %f\n\tRadius: %f\n", circle->cx, circle->cy, circle->r);
        strcat(returnString, otherAttributes);
    }
    else
    {
        sprintf(returnString, "Circle Data:\n\tCenter X-Coordinate: %f\n\tCenter Y-Coordinate: %f\n\tRadius: %f\n\tUnits: %s\n", circle->cx, circle->cy, circle->r, circle->units);
        strcat(returnString, otherAttributes);
    }

    free(otherAttributes);

    return returnString;
}

int compareCircles(const void *first, const void *second)
{
    return 0;
}

/*
All Path Helpers Below
*/

void deletePath(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot delete a NULL Path\n");
        return;
    }

    Path *path = (Path *)data;
    freeList(path->otherAttributes);
    free(path);
}

char *pathToString(void *data)
{
    if (!data)
    {
        fprintf(stderr, "Cannot use NULL Path for toString\n");
        return NULL;
    }

    Path *path = (Path *)data;
    if (!(path->otherAttributes))
    {
        fprintf(stderr, "Cannot use NULL Path other attributes for toString\n");
        return NULL;
    }
    char *otherAttributes = toString(path->otherAttributes);
    if (!otherAttributes)
    {
        fprintf(stderr, "Path to string failed creating attributes to string\n");
        return NULL;
    }

    int memLen = strlen(otherAttributes) + strlen(path->data) + 22;
    char *returnString = (char *)malloc(sizeof(char) * memLen);
    if (!returnString)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    sprintf(returnString, "Path Data:\n\tData: %s\n", path->data);
    strcat(returnString, otherAttributes);

    free(otherAttributes);

    return returnString;
}

int comparePaths(const void *first, const void *second)
{
    return 0;
}

/*
Shaper Getters:
    All getters implement the following algorithm
        - Initialize list of corresponding shape with dummy delete so data CANNOT be mutated/manipulated
        - Iterate through initial SVG shape list and add each shape to new reference list
        - Call the traverse group helper function with the reference list to populate the list
            with all shapes contained in ANY nested groups within the document
        - Return the corresponding shape reference list
*/
List *getRects(const SVG *img)
{
    if (!img)
    {
        fprintf(stderr, "Cannot pass NULL SVG struct\n");
        return NULL;
    }
    else if (!(img->rectangles))
    {
        fprintf(stderr, "Cannot pass NULL SVG rectangles list\n");
        return NULL;
    }

    List *totalRects = initializeList(&rectangleToString, &dummyDeleteFunc, &compareRectangles);
    if (!totalRects)
    {
        fprintf(stderr, "Rectangles List Intialization Failed\n");
        return NULL;
    }

    elementType elemType = RECT;
    ListIterator listLooper;
    listLooper = createIterator(img->rectangles);
    void *currAttr;

    while ((currAttr = nextElement(&listLooper)))
    {
        insertBack(totalRects, currAttr);
    }

    traverseGroups(img->groups, totalRects, elemType);

    return totalRects;
}

List *getCircles(const SVG *img)
{
    if (!img)
    {
        fprintf(stderr, "Cannot pass NULL SVG struct\n");
        return NULL;
    }
    else if (!(img->circles))
    {
        fprintf(stderr, "Cannot pass NULL SVG circles list\n");
        return NULL;
    }

    List *totalCircles = initializeList(&circleToString, &dummyDeleteFunc, &compareCircles);
    if (!totalCircles)
    {
        fprintf(stderr, "Circles List Intialization Failed\n");
        return NULL;
    }

    elementType elemType = CIRC;
    ListIterator listLooper;
    listLooper = createIterator(img->circles);
    void *currAttr;

    while ((currAttr = nextElement(&listLooper)))
    {
        insertBack(totalCircles, currAttr);
    }

    traverseGroups(img->groups, totalCircles, elemType);

    return totalCircles;
}

List *getGroups(const SVG *img)
{
    if (!img)
    {
        fprintf(stderr, "Cannot pass NULL SVG struct\n");
        return NULL;
    }
    else if (!(img->groups))
    {
        fprintf(stderr, "Cannot pass NULL SVG groups list\n");
        return NULL;
    }

    List *totalGroups = initializeList(&groupToString, &dummyDeleteFunc, &compareGroups);
    if (!totalGroups)
    {
        fprintf(stderr, "Groups List Intialization Failed\n");
        return NULL;
    }

    elementType elemType = GROUP;
    ListIterator listLooper;
    listLooper = createIterator(img->groups);
    void *currAttr;

    while ((currAttr = nextElement(&listLooper)))
    {
        insertBack(totalGroups, currAttr);
    }

    traverseGroups(img->groups, totalGroups, elemType);

    return totalGroups;
}

List *getPaths(const SVG *img)
{
    if (!img)
    {
        fprintf(stderr, "Cannot pass NULL SVG struct\n");
        return NULL;
    }
    else if (!(img->paths))
    {
        fprintf(stderr, "Cannot pass NULL SVG paths list\n");
        return NULL;
    }

    List *totalPaths = initializeList(&pathToString, &dummyDeleteFunc, &comparePaths);
    if (!totalPaths)
    {
        fprintf(stderr, "Paths List Intialization Failed\n");
        return NULL;
    }

    elementType elemType = PATH;
    ListIterator listLooper;
    listLooper = createIterator(img->paths);
    void *currAttr;

    while ((currAttr = nextElement(&listLooper)))
    {
        insertBack(totalPaths, currAttr);
    }

    traverseGroups(img->groups, totalPaths, elemType);

    return totalPaths;
}

/*
Data Summary Functions:
    All Summary functions implement the following algorithm
        - Initialize a counter to 0
        - Initialize list a list of all corresponding shapes, using get<Shape>(SVG* img);
        - Iterate through all <Shape> within the returned list
        - Check and compare for corresponding data with each, element type
            i.e. If we are doing rectangles we will calculate and compare areas
            If we are doing paths simply strcasecmp the data
        - If there is a match, add 1 to a counter
        - Once full list has been iterated through return counter
*/

int numRectsWithArea(const SVG *img, float area)
{
    if (!img)
    {
        fprintf(stderr, "Cannot pass NULL SVG struct\n");
        return 0;
    }
    else if (area < 0 || !(img->rectangles))
    {
        fprintf(stderr, "Invalid Parameters numRectsWithArea\n");
        return 0;
    }

    List *totalRects = getRects(img);
    if (!totalRects)
    {
        fprintf(stderr, "Error getting Total Rectangles from SVG\n");
        return 0;
    }

    int counter = 0;
    ListIterator listLooper;
    listLooper = createIterator(totalRects);
    void *currAttr;

    while ((currAttr = nextElement(&listLooper)))
    {
        Rectangle *rect = (Rectangle *)currAttr;
        float currArea = (rect->width * rect->height);
        if (ceil(currArea) == ceil(area))
        {
            counter++;
        }
    }

    freeList(totalRects);

    return counter;
}

int numCirclesWithArea(const SVG *img, float area)
{
    if (!img)
    {
        fprintf(stderr, "Cannot pass NULL SVG struct\n");
        return 0;
    }
    else if (area < 0 || !(img->circles))
    {
        fprintf(stderr, "Invalid Parameters numCirclesWithArea\n");
        return 0;
    }

    List *totalCircles = getCircles(img);
    if (!totalCircles)
    {
        fprintf(stderr, "Error getting Total Circles from SVG\n");
        return 0;
    }

    int counter = 0;
    ListIterator listLooper;
    listLooper = createIterator(totalCircles);
    void *currAttr;

    while ((currAttr = nextElement(&listLooper)))
    {
        Circle *circle = (Circle *)currAttr;
        float currArea = ((circle->r) * (circle->r)) * 3.14159;
        if (ceil(currArea) == ceil(area))
        {
            counter++;
        }
    }

    freeList(totalCircles);

    return counter;
}

int numPathsWithdata(const SVG *img, const char *data)
{
    if (!img)
    {
        fprintf(stderr, "Cannot pass NULL SVG struct\n");
        return 0;
    }
    else if (!data || !(img->paths))
    {
        fprintf(stderr, "Invalid Parameters numPathsWithdata\n");
        return 0;
    }

    List *totalPaths = getPaths(img);
    if (!totalPaths)
    {
        fprintf(stderr, "Error getting Total Paths from SVG\n");
        return 0;
    }

    int counter = 0;
    ListIterator listLooper;
    listLooper = createIterator(totalPaths);
    void *currAttr;

    while ((currAttr = nextElement(&listLooper)))
    {
        Path *pathData = (Path *)currAttr;
        if (strcasecmp(pathData->data, data) == 0)
        {
            counter++;
        }
    }

    freeList(totalPaths);

    return counter;
}

int numGroupsWithLen(const SVG *img, int len)
{
    if (!img)
    {
        fprintf(stderr, "Cannot pass NULL SVG struct\n");
        return 0;
    }
    else if (len < 0 || !(img->groups))
    {
        fprintf(stderr, "Invalid Parameters numGroupsWithLen\n");
        return 0;
    }

    List *totalGroups = getGroups(img);
    if (!totalGroups)
    {
        fprintf(stderr, "Error getting Total Paths from SVG\n");
        return 0;
    }

    int counter = 0;
    ListIterator listLooper;
    listLooper = createIterator(totalGroups);
    void *currAttr;

    while ((currAttr = nextElement(&listLooper)))
    {
        Group *groupNode = (Group *)currAttr;
        int tempCount = 0;
        tempCount += getLength(groupNode->circles);
        tempCount += getLength(groupNode->rectangles);
        tempCount += getLength(groupNode->paths);
        tempCount += getLength(groupNode->groups);
        if (tempCount == len)
        {
            counter++;
        }
    }

    freeList(totalGroups);

    return counter;
}

/*
This summary function is the exception, in this one we use a seperate
helper function to traverse all lists and nested lists within the SVG struct
and simply add the result of any getLength(<Shape>->otherAttributes)
call. Finally the counter is returned which represents all number
of other attributes within the SVG structure.
*/

int numAttr(const SVG *img)
{
    if (!img)
    {
        fprintf(stderr, "Cannot pass NULL SVG struct\n");
        return 0;
    }
    else if (!(img->rectangles) || !(img->circles) || !(img->paths) || !(img->groups) || !(img->otherAttributes))
    {
        fprintf(stderr, "Cannot pass NULL SVG Lists\n");
        return 0;
    }

    List *allRects = getRects(img);
    List *allCircles = getCircles(img);
    List *allPaths = getPaths(img);
    List *allGroups = getGroups(img);
    int counter = 0;

    counter += getLength(img->otherAttributes);
    counter += traverseWholeSVG(allRects, RECT);
    counter += traverseWholeSVG(allCircles, CIRC);
    counter += traverseWholeSVG(allPaths, PATH);
    counter += traverseWholeSVG(allGroups, GROUP);

    freeList(allRects);
    freeList(allCircles);
    freeList(allPaths);
    freeList(allGroups);

    return counter;
}

/*
All creation from file and validation functions below:
*/

SVG *createValidSVG(const char *fileName, const char *schemaFile)
{
    if (!fileName || !schemaFile)
    {
        fprintf(stderr, "Cannnot give a NULL file names, createValidSVG\n");
        return NULL;
    }
    else if (strlen(fileName) <= 0 || strlen(schemaFile) <= 0)
    {
        fprintf(stderr, "Cannnot give an empty file names, createValidSVG\n");
        return NULL;
    }

    SVG *img = NULL;
    xmlDoc *doc;

    LIBXML_TEST_VERSION

    doc = xmlReadFile(fileName, NULL, 0);
    bool isValidTree = validXmlTree(doc, schemaFile);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    if (isValidTree)
    {
        img = createSVG(fileName);
        if (!img)
        {
            fprintf(stderr, "SVG Parse failed, createValidSVG\n");
            return NULL;
        }
        else if (!validateSVGStruct(img))
        {
            fprintf(stderr, "Not a valid SVG struct, createValidSVG\n");
            return NULL;
        }
    }
    else
    {
        fprintf(stderr, "Not a valid xml File, createValidSVG\n");
        return NULL;
    }

    return img;
}

bool writeSVG(const SVG *img, const char *fileName)
{
    if (!img || !fileName)
    {
        fprintf(stderr, "SVG or fileName cannot be NULL writeSVG\n");
        return false;
    }
    else if (strlen(fileName) <= 0 || (!(strstr(fileName, ".svg")) && !(strstr(fileName, ".SVG"))))
    {
        fprintf(stderr, "Invalid file name writeSVG\n");
        return false;
    }

    LIBXML_TEST_VERSION;

    xmlDoc *doc = SVGtoXmlDoc(img);

    if (!doc)
    {
        fprintf(stderr, "error occured calling SVGtoXmlDoc\n");
        return false;
    }

    int status = xmlSaveFormatFileEnc(fileName, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    if (status == -1)
    {
        fprintf(stderr, "Error with exporting xmlDoc to File\n");
        return false;
    }

    return true;
}

bool validateSVG(const SVG *img, const char *schemaFile)
{
    if (!img || !schemaFile || strlen(schemaFile) <= 0)
    {
        fprintf(stderr, "Invalid params validateSVG\n");
        return false;
    }

    if (!validateSVGStruct(img))
    {
        return false;
    }

    LIBXML_TEST_VERSION;

    xmlDoc *doc = SVGtoXmlDoc(img);
    if (!doc)
    {
        fprintf(stderr, "error occured calling SVGtoXmlDoc\n");
        return false;
    }

    bool isValidTree = validXmlTree(doc, schemaFile);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    if (!isValidTree)
    {
        return false;
    }

    return true;
}

bool setAttribute(SVG *img, elementType elemType, int elemIndex, Attribute *newAttribute)
{
    if (!img || !newAttribute)
    {
        fprintf(stderr, "Cannot pass NULL arguments setAttribute\n");
        return false;
    }
    else if (!(img->otherAttributes) || !(img->rectangles) || !(img->circles) || !(img->groups) || !(img->paths) || !(newAttribute->name) || !(newAttribute->value) || strlen(img->namespace) <= 0)
    {
        fprintf(stderr, "Invalid Parameters setAttribute\n");
        return false;
    }
    else if (strlen(newAttribute->name) <= 0 || strlen(newAttribute->value) <= 0)
    {
        fprintf(stderr, "Cannot have empty data values in attribute setAttribute\n");
        return false;
    }

    if (elemType == SVG_IMG && (strcasecmp(newAttribute->name, "desc") == 0 || strcasecmp(newAttribute->name, "title") == 0))
    {
        if (strlen(newAttribute->value) >= 256)
        {
            if (strcasecmp(newAttribute->name, "desc") == 0)
            {
                strncpy(img->description, newAttribute->value, 255);
                img->description[255] = '\0';
                deleteAttribute(newAttribute);
                return true;
            }
            strncpy(img->title, newAttribute->value, 255);
            img->title[255] = '\0';
            deleteAttribute(newAttribute);
            return true;
        }
        else if (strlen(newAttribute->value) >= 0)
        {
            if (strcasecmp(newAttribute->name, "desc") == 0)
            {
                strcpy(img->description, newAttribute->value);
                deleteAttribute(newAttribute);
                return true;
            }
            strcpy(img->title, newAttribute->value);
            deleteAttribute(newAttribute);
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (elemType == SVG_IMG)
    {
        int tempStatus = setOtherAttr(img->otherAttributes, newAttribute);
        if (tempStatus == -1)
        {
            fprintf(stderr, "failed in adding/finding other attribute setAttribute\n");
            return false;
        }
        else if (tempStatus == 1)
        {
            deleteAttribute(newAttribute);
            return true;
        }
        else
        {
            return true;
        }
    }

    List *pointerToList;
    switch (elemType)
    {
    case CIRC:
        pointerToList = img->circles;
        break;
    case RECT:
        pointerToList = img->rectangles;
        break;
    case PATH:
        pointerToList = img->paths;
        break;
    case GROUP:
        pointerToList = img->groups;
        break;
    default:
        fprintf(stderr, "Invalid elemType in setAttribute\n");
        return false;
    }

    if (elemIndex >= getLength(pointerToList))
    {
        fprintf(stderr, "Index out of Bound setAttribute\n");
        return false;
    }

    ListIterator counter = createIterator(pointerToList);
    void *data;
    for (int i = 0; i < elemIndex + 1; i++)
    {
        data = nextElement(&counter);
    }

    int status = 0;
    switch (elemType)
    {
    case CIRC:
        status = setCircAttr(data, newAttribute);
        break;
    case RECT:
        status = setRectAttr(data, newAttribute);
        break;
    case PATH:
        status = setPathAttr(img->paths, newAttribute, elemIndex);
        break;
    case GROUP:
        status = setGroupAttr(data, newAttribute);
        break;
    default:
        fprintf(stderr, "Invalid elemType in setAttribute\n");
        return false;
    }

    if (status == -1)
    {
        fprintf(stderr, "Error occured adding/editing attribute\n");
        return false;
    }
    else if (status == 1)
    {
        deleteAttribute(newAttribute);
        return true;
    }

    return true;
}

void addComponent(SVG *img, elementType type, void *newElement)
{
    if (!img || !newElement)
    {
        fprintf(stderr, "Cannot pass NULL arguments addComponent\n");
        return;
    }
    else if (!(img->rectangles) || !(img->circles) || !(img->paths) || !(img->groups) || !(img->otherAttributes))
    {
        fprintf(stderr, "Cannot pass NULL arguments addComponent\n");
        return;
    }

    if (type == RECT)
    {
        Rectangle *rect = (Rectangle *)newElement;
        if (rect->width < 0 || rect->height < 0 || !(rect->otherAttributes))
        {
            fprintf(stderr, "Invalid rectangle type parameters to add\n");
            return;
        }
        insertBack(img->rectangles, newElement);
    }
    else if (type == CIRC)
    {
        Circle *circ = (Circle *)newElement;
        if (circ->r < 0 || !(circ->otherAttributes))
        {
            fprintf(stderr, "Invalid circle type parameters to add\n");
            return;
        }
        insertBack(img->circles, newElement);
    }
    else if (type == PATH)
    {
        Path *currPath = (Path *)newElement;
        if (!(currPath->otherAttributes) || !(currPath->data))
        {
            fprintf(stderr, "Invalid path type parameters to add\n");
            return;
        }
        insertBack(img->paths, newElement);
    }
}

/*
All shape or structure to JSON below:
*/

char *attrToJSON(const Attribute *a)
{
    char *str;

    if (!a || !(a->name))
    {
        fprintf(stderr, "Cannot pass NULL Attribute attrToSJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "{}");
        return str;
    }

    int memLen = strlen(a->name) + strlen(a->value) + 23;
    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }
    strcpy(str, "{\"name\":\"");
    strcat(str, a->name);
    strcat(str, "\",\"value\":\"");
    strcat(str, a->value);
    strcat(str, "\"}");

    return str;
}

char *circleToJSON(const Circle *c)
{
    char *str;

    if (!c || !(c->otherAttributes))
    {
        fprintf(stderr, "Cannot pass NULL Circle circleToJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "{}");
        return str;
    }

    char buffers[4][200];

    sprintf(buffers[0], "%0.2f", c->cx);
    sprintf(buffers[1], "%0.2f", c->cy);
    sprintf(buffers[2], "%0.2f", c->r);
    sprintf(buffers[3], "%d", getLength(c->otherAttributes));

    int memLen = strlen(buffers[0]) + strlen(buffers[1]) + strlen(buffers[2]) + strlen(buffers[3]) + strlen(c->units) + 41;
    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    strcpy(str, "{\"cx\":");
    strcat(str, buffers[0]);
    strcat(str, ",\"cy\":");
    strcat(str, buffers[1]);
    strcat(str, ",\"r\":");
    strcat(str, buffers[2]);
    strcat(str, ",\"numAttr\":");
    strcat(str, buffers[3]);
    strcat(str, ",\"units\":\"");
    strcat(str, c->units);
    strcat(str, "\"}");

    return str;
}

char *rectToJSON(const Rectangle *r)
{
    char *str;

    if (!r || !(r->otherAttributes))
    {
        fprintf(stderr, "Cannot pass NULL Rectangle rectToJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "{}");
        return str;
    }

    char buffers[5][200];

    sprintf(buffers[0], "%0.2f", r->x);
    sprintf(buffers[1], "%0.2f", r->y);
    sprintf(buffers[2], "%0.2f", r->width);
    sprintf(buffers[3], "%0.2f", r->height);
    sprintf(buffers[4], "%d", getLength(r->otherAttributes));

    int memLen = strlen(buffers[0]) + strlen(buffers[1]) + strlen(buffers[2]) + strlen(buffers[3]) + strlen(buffers[4]) + strlen(r->units) + 44;
    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    strcpy(str, "{\"x\":");
    strcat(str, buffers[0]);
    strcat(str, ",\"y\":");
    strcat(str, buffers[1]);
    strcat(str, ",\"w\":");
    strcat(str, buffers[2]);
    strcat(str, ",\"h\":");
    strcat(str, buffers[3]);
    strcat(str, ",\"numAttr\":");
    strcat(str, buffers[4]);
    strcat(str, ",\"units\":\"");
    strcat(str, r->units);
    strcat(str, "\"}");

    return str;
}

char *pathToJSON(const Path *p)
{
    char *str;

    if (!p || !(p->otherAttributes) || !(p->data))
    {
        fprintf(stderr, "Cannot pass NULL Path pathToJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "{}");
        return str;
    }

    char buffer[100];
    sprintf(buffer, "%d", getLength(p->otherAttributes));

    int memLen = strlen(buffer) + 84;
    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    strcpy(str, "{\"d\":\"");
    strncat(str, p->data, 64);
    strcat(str, "\",\"numAttr\":");
    strcat(str, buffer);
    strcat(str, "}");

    return str;
}

char *groupToJSON(const Group *g)
{
    char *str;

    if (!g || !(g->otherAttributes) || !(g->rectangles) || !(g->circles) || !(g->paths) || !(g->groups))
    {
        fprintf(stderr, "Cannot pass NULL Group groupToJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "{}");
        return str;
    }

    char buffer[2][200];
    sprintf(buffer[0], "%d", (getLength(g->rectangles) + getLength(g->circles) + getLength(g->paths) + getLength(g->groups)));
    sprintf(buffer[1], "%d", getLength(g->otherAttributes));

    int memLen = strlen(buffer[0]) + strlen(buffer[1]) + 25;
    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    strcpy(str, "{\"children\":");
    strcat(str, buffer[0]);
    strcat(str, ",\"numAttr\":");
    strcat(str, buffer[1]);
    strcat(str, "}");

    return str;
}

char *attrListToJSON(const List *list)
{
    char *str;

    if (!list || getLength((List *)list) <= 0)
    {
        fprintf(stderr, "Cannot pass NULL List attrListToJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "[]");
        return str;
    }

    int numAttr = getLength((List *)list);

    ListIterator iterator = createIterator((List *)list);
    Attribute *data;
    int counter = 0;
    int memLen = 3 + (numAttr - 1);

    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }
    strcpy(str, "[");

    while ((data = (Attribute *)nextElement(&iterator)) != NULL)
    {
        counter++;
        char *temp = attrToJSON(data);
        memLen += strlen(temp);
        str = realloc(str, sizeof(char) * memLen);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcat(str, temp);
        if (counter != (numAttr))
        {
            strcat(str, ",");
        }
        free(temp);
    }
    strcat(str, "]");

    return str;
}

char *circListToJSON(const List *list)
{
    char *str;

    if (!list || getLength((List *)list) <= 0)
    {
        fprintf(stderr, "Cannot pass NULL List circListToJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "[]");
        return str;
    }

    int numAttr = getLength((List *)list);

    ListIterator iterator = createIterator((List *)list);
    Circle *data;
    int counter = 0;
    int memLen = 3 + (numAttr - 1);

    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }
    strcpy(str, "[");

    while ((data = (Circle *)nextElement(&iterator)) != NULL)
    {
        counter++;
        char *temp = circleToJSON(data);
        memLen += strlen(temp);
        str = realloc(str, sizeof(char) * memLen);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcat(str, temp);
        if (counter != (numAttr))
        {
            strcat(str, ",");
        }
        free(temp);
    }
    strcat(str, "]");

    return str;
}

char *rectListToJSON(const List *list)
{
    char *str;

    if (!list || getLength((List *)list) <= 0)
    {
        fprintf(stderr, "Cannot pass NULL List rectListToJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "[]");
        return str;
    }

    int numAttr = getLength((List *)list);

    ListIterator iterator = createIterator((List *)list);
    Rectangle *data;
    int counter = 0;
    int memLen = 3 + (numAttr - 1);

    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }
    strcpy(str, "[");

    while ((data = (Rectangle *)nextElement(&iterator)) != NULL)
    {
        counter++;
        char *temp = rectToJSON(data);
        memLen += strlen(temp);
        str = realloc(str, sizeof(char) * memLen);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcat(str, temp);
        if (counter != (numAttr))
        {
            strcat(str, ",");
        }
        free(temp);
    }
    strcat(str, "]");

    return str;
}

char *pathListToJSON(const List *list)
{
    char *str;

    if (!list || getLength((List *)list) <= 0)
    {
        fprintf(stderr, "Cannot pass NULL List pathListToJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "[]");
        return str;
    }

    int numAttr = getLength((List *)list);

    ListIterator iterator = createIterator((List *)list);
    Path *data;
    int counter = 0;
    int memLen = 3 + (numAttr - 1);

    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }
    strcpy(str, "[");

    while ((data = (Path *)nextElement(&iterator)) != NULL)
    {
        counter++;
        char *temp = pathToJSON(data);
        memLen += strlen(temp);
        str = realloc(str, sizeof(char) * memLen);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcat(str, temp);
        if (counter != (numAttr))
        {
            strcat(str, ",");
        }
        free(temp);
    }
    strcat(str, "]");

    return str;
}

char *groupListToJSON(const List *list)
{
    char *str;

    if (!list || getLength((List *)list) <= 0)
    {
        fprintf(stderr, "Cannot pass NULL List groupListToJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "[]");
        return str;
    }

    int numAttr = getLength((List *)list);

    ListIterator iterator = createIterator((List *)list);
    Group *data;
    int counter = 0;
    int memLen = 3 + (numAttr - 1);

    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }
    strcpy(str, "[");

    while ((data = (Group *)nextElement(&iterator)) != NULL)
    {
        counter++;
        char *temp = groupToJSON(data);
        memLen += strlen(temp);
        str = realloc(str, sizeof(char) * memLen);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcat(str, temp);
        if (counter != (numAttr))
        {
            strcat(str, ",");
        }
        free(temp);
    }
    strcat(str, "]");

    return str;
}

char *SVGtoJSON(const SVG *img)
{
    char *str;

    if (!img || !(img->rectangles) || !(img->circles) || !(img->paths) || !(img->groups))
    {
        fprintf(stderr, "Cannot pass NULL SVG SVGtoJSON\n");
        str = (char *)malloc(sizeof(char) * 3);
        if (!str)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(str, "{}");
        return str;
    }

    List *rects = getRects(img);
    List *circles = getCircles(img);
    List *paths = getPaths(img);
    List *groups = getGroups(img);

    char buffer[4][200];
    sprintf(buffer[0], "%d", (getLength(rects)));
    sprintf(buffer[1], "%d", (getLength(circles)));
    sprintf(buffer[2], "%d", (getLength(paths)));
    sprintf(buffer[3], "%d", (getLength(groups)));

    freeList(rects);
    freeList(circles);
    freeList(paths);
    freeList(groups);

    int memLen = strlen(buffer[0]) + strlen(buffer[1]) + strlen(buffer[2]) + strlen(buffer[3]) + 49;
    str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    strcpy(str, "{\"numRect\":");
    strcat(str, buffer[0]);
    strcat(str, ",\"numCirc\":");
    strcat(str, buffer[1]);
    strcat(str, ",\"numPaths\":");
    strcat(str, buffer[2]);
    strcat(str, ",\"numGroups\":");
    strcat(str, buffer[3]);
    strcat(str, "}");

    return str;
}

/*
All reverse conversions below:
*/

SVG *JSONtoSVG(const char *svgString)
{
    if (!svgString || strlen(svgString) <= 0)
    {
        fprintf(stderr, "Invalid param JSONtoSVG\n");
        return NULL;
    }

    SVG *img = (SVG *)malloc(sizeof(SVG));
    img->title[0] = '\0';
    img->description[0] = '\0';
    img->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
    img->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
    img->paths = initializeList(&pathToString, &deletePath, &comparePaths);
    img->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
    img->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
    if (!(img->groups) || !(img->paths) || !(img->circles) || !(img->rectangles) || !(img->otherAttributes))
    {
        fprintf(stderr, "SVG List initializations failed JSONtoSVG\n");
        return NULL;
    }

    strcpy(img->namespace, "http://www.w3.org/2000/svg");
    char *temp = strtok((char *)svgString, ",:\"{}");
    while (temp != NULL)
    {
        if (strcasecmp(temp, "descr") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp != NULL && strcasecmp(temp, "title") != 0)
            {
                if (strlen(temp) >= 256)
                {
                    strncpy(img->description, temp, 255);
                    img->description[255] = '\0';
                }
                else if (strlen(temp) >= 0)
                {
                    strcpy(img->description, temp);
                }
            }
            continue;
        }
        if (strcasecmp(temp, "title") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp != NULL && strcasecmp(temp, "descr") != 0)
            {
                if (strlen(temp) >= 256)
                {
                    strncpy(img->title, temp, 255);
                    img->title[255] = '\0';
                }
                else if (strlen(temp) >= 0)
                {
                    strcpy(img->title, temp);
                }
            }
            continue;
        }
        temp = strtok(NULL, ",:\"{}");
    }

    return img;
}

Rectangle *JSONtoRect(const char *svgString)
{
    if (!svgString || strlen(svgString) <= 0)
    {
        fprintf(stderr, "Invalid param JSONtoRect\n");
        return NULL;
    }

    Rectangle *rect = (Rectangle *)malloc(sizeof(Rectangle));
    rect->units[0] = '\0';
    rect->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
    if (!(rect->otherAttributes))
    {
        fprintf(stderr, "Rect List initializations failed JSONtoRect\n");
        return NULL;
    }

    char *temp = strtok((char *)svgString, ",:\"{}");
    while (temp != NULL)
    {
        if (strcasecmp(temp, "x") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp)
            {
                rect->x = atof(temp);
                continue;
            }
        }
        else if (strcasecmp(temp, "y") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp)
            {
                rect->y = atof(temp);
                continue;
            }
        }
        else if (strcasecmp(temp, "w") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp)
            {
                rect->width = atof(temp);
                continue;
            }
        }
        else if (strcasecmp(temp, "h") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp)
            {
                rect->height = atof(temp);
                continue;
            }
        }
        else if (strcasecmp(temp, "units") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp)
            {
                strcpy(rect->units, temp);
                continue;
            }
        }
        temp = strtok(NULL, ",:\"{}");
    }

    return rect;
}

Circle *JSONtoCircle(const char *svgString)
{
    if (!svgString || strlen(svgString) <= 0)
    {
        fprintf(stderr, "Invalid param JSONtoCircle\n");
        return NULL;
    }

    Circle *circle = (Circle *)malloc(sizeof(Circle));
    circle->units[0] = '\0';
    circle->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
    if (!(circle->otherAttributes))
    {
        fprintf(stderr, "Circle List initializations failed JSONtoCircle\n");
        return NULL;
    }

    char *temp = strtok((char *)svgString, ",:\"{}");
    while (temp != NULL)
    {
        if (strcasecmp(temp, "cx") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp)
            {
                circle->cx = atof(temp);
                continue;
            }
        }
        else if (strcasecmp(temp, "cy") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp)
            {
                circle->cy = atof(temp);
                continue;
            }
        }
        else if (strcasecmp(temp, "r") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp)
            {
                circle->r = atof(temp);
                continue;
            }
        }
        else if (strcasecmp(temp, "units") == 0)
        {
            temp = strtok(NULL, ",:\"{}");
            if (temp)
            {
                strcpy(circle->units, temp);
                continue;
            }
        }
        temp = strtok(NULL, ",:\"{}");
    }

    return circle;
}

/*
JSON helpers below:
*/

char *getDescription(char *fileName, char *schemaFile)
{
    char *string = NULL;
    SVG *img = createValidSVG(fileName, schemaFile);

    if (!img)
    {
        string = malloc(sizeof(char) * 3);
        strcpy(string, "{}");
    }
    else
    {
        int memLen = strlen(img->description) + 1;
        string = malloc(sizeof(char) * memLen);
        strcpy(string, img->description);
        deleteSVG(img);
    }

    return string;
}

char *getTitle(char *fileName, char *schemaFile)
{
    char *string = NULL;
    SVG *img = createValidSVG(fileName, schemaFile);

    if (!img)
    {
        string = malloc(sizeof(char) * 3);
        strcpy(string, "{}");
    }
    else
    {
        int memLen = strlen(img->title) + 1;
        string = malloc(sizeof(char) * memLen);
        strcpy(string, img->title);
        deleteSVG(img);
    }

    return string;
}

char *rectListToJSONFromSVGFile(char *fileName, char *schemaFile)
{
    char *string = NULL;
    SVG *img = createValidSVG(fileName, schemaFile);

    if (!img)
    {
        string = malloc(sizeof(char) * 3);
        strcpy(string, "[]");
    }
    else
    {
        string = rectListToJSON(img->rectangles);
        deleteSVG(img);
    }

    return string;
}

char *circleListToJSONFromSVGFile(char *fileName, char *schemaFile)
{
    char *string = NULL;
    SVG *img = createValidSVG(fileName, schemaFile);

    if (!img)
    {
        string = malloc(sizeof(char) * 3);
        strcpy(string, "[]");
    }

    else
    {
        string = circListToJSON(img->circles);
        deleteSVG(img);
    }

    return string;
}

char *pathListToJSONFromSVGFile(char *fileName, char *schemaFile)
{
    char *string = NULL;
    SVG *img = createValidSVG(fileName, schemaFile);

    if (!img)
    {
        string = malloc(sizeof(char) * 3);
        strcpy(string, "[]");
    }
    else
    {
        string = pathListToJSON(img->paths);
        deleteSVG(img);
    }

    return string;
}

char *groupListToJSONFromSVGFile(char *fileName, char *schemaFile)
{
    char *string = NULL;
    SVG *img = createValidSVG(fileName, schemaFile);

    if (!img)
    {
        string = malloc(sizeof(char) * 3);
        strcpy(string, "[]");
    }
    else
    {
        string = groupListToJSON(img->groups);
        deleteSVG(img);
    }

    return string;
}

char *SVGtoJSONFromSVGFile(char *fileName, char *schemaFile)
{
    char *string = NULL;
    SVG *img = createValidSVG(fileName, schemaFile);
    if (!img)
    {
        string = malloc(sizeof(char) * 3);
        strcpy(string, "{}");
    }
    else
    {
        string = SVGtoJSON(img);
        deleteSVG(img);
    }

    return string;
}

int userCreatedSVGFunction(char *fileName, char *schemaFile, char *title, char *desc)
{
    int status = 0;
    if (strlen(title) >= 256 || strlen(desc) >= 256)
    {
        return status;
    }

    SVG *img = (SVG *)malloc(sizeof(SVG));
    int memLen = strlen(fileName) + 5;
    char *newFilename = malloc(sizeof(char *) * memLen);
    strcpy(newFilename, fileName);

    strcpy(img->namespace, "http://www.w3.org/2000/svg");
    strcpy(img->title, title);
    strcpy(img->description, desc);
    img->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
    img->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
    img->paths = initializeList(&pathToString, &deletePath, &comparePaths);
    img->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
    img->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

    if (strstr(fileName, ".svg") == NULL)
    {
        memLen += 4;
        newFilename = realloc(newFilename, sizeof(char) * memLen);
        strcat(newFilename, ".svg");
    }

    if (validateSVG(img, schemaFile))
    {
        if (writeSVG(img, newFilename))
        {
            status = 1;
        }
    }

    deleteSVG(img);
    free(newFilename);
    return status;
}

int changeSVGTitle(char *fileName, char *schemaFile, char *currTitle)
{
    int status = 0;
    if (strlen(currTitle) >= 256)
    {
        return status;
    }
    SVG *img = createValidSVG(fileName, schemaFile);

    if (img && currTitle)
    {
        strncpy(img->title, currTitle, 255);
        img->title[255] = '\0';
        writeSVG(img, fileName);
        deleteSVG(img);
        status = 1;
    }
    else
    {
        fprintf(stderr, "invalid changeSVGTitle\n");
    }

    return status;
}

int changeSVGDescription(char *fileName, char *schemaFile, char *currDesc)
{
    int status = 0;
    if (strlen(currDesc) >= 256)
    {
        return status;
    }

    SVG *img = createValidSVG(fileName, schemaFile);

    if (img && currDesc)
    {
        strncpy(img->description, currDesc, 255);
        img->description[255] = '\0';
        writeSVG(img, fileName);
        deleteSVG(img);
        status = 1;
    }
    else
    {
        fprintf(stderr, "invalid changeSVGDescription\n");
    }

    return status;
}

int addRectToSVG(char *fileName, char *schemaFile, float x, float y, float width, float height, char *units)
{
    SVG *img = createValidSVG(fileName, schemaFile);
    int status = 0;

    if (width < 0 || height < 0 || !img || strlen(units) >= 50)
    {
        deleteSVG(img);
        return status;
    }

    Rectangle *rect = malloc(sizeof(Rectangle));
    rect->x = x;
    rect->y = y;
    rect->width = width;
    rect->height = height;
    strcpy(rect->units, units);
    rect->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

    insertBack(img->rectangles, (void *)rect);

    if (validateSVG(img, schemaFile))
    {
        if (writeSVG(img, fileName))
        {
            status = 1;
        }
    }

    deleteSVG(img);
    return status;
}

int addCircleToSVG(char *fileName, char *schemaFile, float x, float y, float r, char *units)
{
    SVG *img = createValidSVG(fileName, schemaFile);
    int status = 0;

    if (r < 0 || !img || strlen(units) >= 50)
    {
        deleteSVG(img);
        return status;
    }

    Circle *circ = malloc(sizeof(Circle));
    circ->cx = x;
    circ->cy = y;
    circ->r = r;
    strcpy(circ->units, units);
    circ->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

    insertBack(img->circles, (void *)circ);

    if (validateSVG(img, schemaFile))
    {
        if (writeSVG(img, fileName))
        {
            status = 1;
        }
    }

    deleteSVG(img);
    return status;
}

int scaleShape(char *fileName, char *schemaFile, char *shapeType, float scaleFactor)
{
    int status = 0;
    SVG *img = createValidSVG(fileName, schemaFile);
    List *shapeList;
    ListIterator count;

    if (!img)
    {
        fprintf(stderr, "Error with creating SVG scaleShape\n");
        return status;
    }

    if (strcasecmp(shapeType, "rectangle") == 0 || strcasecmp(shapeType, "rect") == 0)
    {
        void *data;
        shapeList = getRects(img);
        count = createIterator(shapeList);
        while ((data = nextElement(&count)) != NULL)
        {
            Rectangle *rect = (Rectangle *)data;
            rect->width = (rect->width) * scaleFactor;
            rect->height = (rect->height) * scaleFactor;
        }
    }

    else if (strcasecmp(shapeType, "circle") == 0 || strcasecmp(shapeType, "circ") == 0)
    {
        void *data;
        shapeList = getCircles(img);
        count = createIterator(shapeList);
        while ((data = nextElement(&count)) != NULL)
        {
            Circle *circle = (Circle *)data;
            circle->r = (circle->r) * scaleFactor;
        }
    }

    else
    {
        deleteSVG(img);
        return status;
    }

    if (validateSVG(img, schemaFile))
    {
        if (writeSVG(img, fileName))
        {
            status = 1;
        }
        else
        {
            fprintf(stderr, "Write failed\n");
        }
    }

    deleteSVG(img);

    return status;
}

int validSVGFile(char *fileName, char *schemaFile)
{
    int status = 0;
    SVG *img = createValidSVG(fileName, schemaFile);
    if (!img)
    {
        return status;
    }
    bool result = validateSVG(img, schemaFile);
    if (result == true)
    {
        status = 1;
    }

    deleteSVG(img);

    return status;
}

char *otherAttributesFromShape(char *fileName, char *schemaFile, char *shape, int shapeNum)
{
    char *data = NULL;
    if (shapeNum < 1)
    {
        data = (char *)malloc(sizeof(char) * 3);
        strcpy(data, "[]");
        return data;
    }

    SVG *img = createValidSVG(fileName, schemaFile);
    ListIterator count;
    void *temp;
    int i;

    if (strcasecmp(shape, "rectangle") == 0 || strcasecmp(shape, "rect") == 0)
    {
        if (shapeNum > getLength(img->rectangles))
        {
            data = (char *)malloc(sizeof(char) * 3);
            strcpy(data, "[]");
            deleteSVG(img);
            return data;
        }

        count = createIterator(img->rectangles);
        for (i = 0; i < shapeNum; i++)
        {
            temp = nextElement(&count);
        }

        Rectangle *tempShape = (Rectangle *)temp;
        data = attrListToJSON(tempShape->otherAttributes);
    }

    else if (strcasecmp(shape, "circle") == 0 || strcasecmp(shape, "circ") == 0)
    {
        if (shapeNum > getLength(img->circles))
        {
            data = (char *)malloc(sizeof(char) * 3);
            strcpy(data, "[]");
            deleteSVG(img);
            return data;
        }

        count = createIterator(img->circles);
        for (i = 0; i < shapeNum; i++)
        {
            temp = nextElement(&count);
        }

        Circle *tempShape = (Circle *)temp;
        data = attrListToJSON(tempShape->otherAttributes);
    }

    else if (strcasecmp(shape, "path") == 0 || strcasecmp(shape, "paths") == 0)
    {
        if (shapeNum > getLength(img->paths))
        {
            data = (char *)malloc(sizeof(char) * 3);
            strcpy(data, "[]");
            deleteSVG(img);
            return data;
        }

        count = createIterator(img->paths);
        for (i = 0; i < shapeNum; i++)
        {
            temp = nextElement(&count);
        }

        Path *tempShape = (Path *)temp;
        data = attrListToJSON(tempShape->otherAttributes);
    }

    else if (strcasecmp(shape, "group") == 0 || strcasecmp(shape, "groups") == 0)
    {
        if (shapeNum > getLength(img->groups))
        {
            data = (char *)malloc(sizeof(char) * 3);
            strcpy(data, "[]");
            deleteSVG(img);
            return data;
        }

        count = createIterator(img->groups);
        for (i = 0; i < shapeNum; i++)
        {
            temp = nextElement(&count);
        }

        Group *tempShape = (Group *)temp;
        data = attrListToJSON(tempShape->otherAttributes);
    }

    else
    {
        data = (char *)malloc(sizeof(char) * 3);
        strcpy(data, "[]");
        deleteSVG(img);
        return data;
    }

    deleteSVG(img);
    return data;
}

int setAttributeInFile(char *fileName, char *schemaFile, char *shape, int shapeNum, char *attrName, char *attrValue)
{
    int status = 0;
    SVG *img = createValidSVG(fileName, schemaFile);
    if (!img)
    {
        fprintf(stderr, "Creating svg failed\n");
    }

    Attribute *attrToAdd = malloc(sizeof(Attribute) + (sizeof(char) * (strlen(attrValue) + 1)));
    attrToAdd->name = malloc(sizeof(char) + (strlen(attrName) + 1));
    elementType type;
    int index = shapeNum - 1;

    strcpy(attrToAdd->name, attrName);
    strcpy(attrToAdd->value, attrValue);

    if (strcasecmp(shape, "rectangle") == 0 || strcasecmp(shape, "rect") == 0)
    {
        type = RECT;
    }
    else if (strcasecmp(shape, "circle") == 0 || strcasecmp(shape, "circ") == 0)
    {
        type = CIRC;
    }
    else if (strcasecmp(shape, "path") == 0 || strcasecmp(shape, "paths") == 0)
    {
        type = PATH;
    }
    else if (strcasecmp(shape, "group") == 0 || strcasecmp(shape, "groups") == 0)
    {
        type = GROUP;
    }
    else
    {
        deleteSVG(img);
        deleteAttribute(attrToAdd);
        return status;
    }

    if (setAttribute(img, type, index, attrToAdd))
    {
        if (validateSVG(img, schemaFile))
        {
            if (writeSVG(img, fileName))
            {
                status = 1;
            }
            else
            {
                deleteAttribute(attrToAdd);
            }
        }
        else
        {
            deleteAttribute(attrToAdd);
        }
    }
    else
    {
        deleteAttribute(attrToAdd);
    }

    deleteSVG(img);
    return status;
}

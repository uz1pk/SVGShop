/**
 * @file SVGHelper.c
 * @author Usman Zaheer (Student# 1148583)
 * @date February 2021
 * @brief File containing the helper functions used by SVGParser.c
 */

#include "SVGParser.h"
#include "SVGHelper.h"

/*
Element Parser Functions:
*/

int parseTree(SVG *finishedTree, xmlNode *prevNode)
{
    xmlNode *currNode = NULL; // node for loop
    int status;               // indicates status of traversal to check later

    for (currNode = prevNode; currNode != NULL; currNode = currNode->next)
    {
        char *name = (char *)currNode->name;

        if (strcasecmp(name, "svg") == 0)
        {
            finishedTree->otherAttributes = parseAttributes(currNode);
            if (!(finishedTree->otherAttributes))
            {
                fprintf(stderr, "SVG Attribute parse failed parseTree\n");
                return -1;
            }
        }

        else if (strcasecmp(name, "title") == 0)
        {
            xmlNode *temp = currNode->children;
            if (temp)
            {
                if ((temp->content))
                {
                    if (strlen((char *)temp->content) >= 256)
                    {
                        strncpy(finishedTree->title, (char *)temp->content, 255);
                        finishedTree->title[255] = '\0';
                    }
                    else if (strlen((char *)temp->content) >= 0)
                    {
                        strcpy(finishedTree->title, (char *)temp->content);
                    }
                }
            }
        }

        else if (strcasecmp(name, "desc") == 0)
        {
            xmlNode *temp = currNode->children;
            if (temp)
            {
                if (temp->content)
                {
                    if (strlen((char *)temp->content) >= 256)
                    {
                        strncpy(finishedTree->description, (char *)temp->content, 255);
                        finishedTree->description[255] = '\0';
                    }
                    else if (strlen((char *)temp->content) >= 0)
                    {
                        strcpy(finishedTree->description, (char *)temp->content);
                    }
                }
            }
        }

        else if (strcasecmp(name, "g") == 0)
        {
            Group *currGroup = parseGroup(currNode);
            if (!currGroup)
            {
                fprintf(stderr, "Layer 1 Group Parse Failed parseTree\n");
                return -1;
            }
            else
            {
                insertBack(finishedTree->groups, (void *)currGroup);
            }
        }

        else if (strcasecmp(name, "path") == 0)
        {
            Path *currPath = parsePath(currNode);
            if (!currPath)
            {
                fprintf(stderr, "Path parse failed parseTree\n");
                return -1;
            }
            else
            {
                insertBack(finishedTree->paths, (void *)currPath);
            }
        }

        else if (strcasecmp(name, "circle") == 0)
        {
            Circle *currCircle = parseCircle(currNode);
            if (!currCircle)
            {
                fprintf(stderr, "Circle parse failed parseTree\n");
                return -1;
            }
            else
            {
                insertBack(finishedTree->circles, (void *)currCircle);
            }
        }

        else if (strcasecmp(name, "rect") == 0)
        {
            Rectangle *currRect = parseRect(currNode);
            if (!currRect)
            {
                fprintf(stderr, "Rectangle parse failed\n");
                return -1;
            }
            else
            {
                insertBack(finishedTree->rectangles, (void *)currRect);
            }
        }

        if (strcasecmp(name, "svg") == 0)
        {
            status = parseTree(finishedTree, currNode->children);
            if (status == -1)
            {
                fprintf(stderr, "Tree traversal failed\n");
                return -1;
            } // ALL elements will be a child of SVG
            // or a group within SVG, which is why it is only recursively done once
        }
    }
    return 0;
}

List *parseAttributes(xmlNode *node)
{
    xmlAttr *attr;                                                                                  // loop attribute node
    List *allAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes); // initialize attributes list
    Attribute *tempAttr;                                                                            // attribute to be added

    if (!allAttributes)
    {
        fprintf(stderr, "Attribute list initialization failed parseAttributes\n");
        return NULL;
    }

    // Loop through every single attribute of the given xmlNode
    for (attr = node->properties; attr != NULL; attr = attr->next)
    {
        xmlNode *value = attr->children;
        char *attributeName = (char *)attr->name;
        char *attributeValue = (char *)value->content;

        /*
        The below is calculating the sizes necessary for creating and initializing
        an Attribute struct pointer thats to be added
        */
        int valueMemLen = strlen(attributeValue) + 1;
        tempAttr = (Attribute *)malloc(sizeof(Attribute) + sizeof(char) * valueMemLen);
        if (!tempAttr)
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }

        int nameMemLen = strlen(attributeName) + 1;
        tempAttr->name = (char *)malloc(sizeof(char) * nameMemLen);
        if (!(tempAttr->name))
        {
            fprintf(stderr, "Malloc error occured\n");
            return NULL;
        }
        strcpy(tempAttr->name, attributeName);

        if (attributeValue)
        {
            strcpy(tempAttr->value, attributeValue);
        }

        insertBack(allAttributes, (void *)tempAttr);
    }

    // return a linked list of all attributes in order (FCFS processing)
    return allAttributes;
}

Group *parseGroup(xmlNode *groupNode)
{
    Group *returnGroup = (Group *)malloc(sizeof(Group));
    if (!returnGroup)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }
    xmlNode *currNode;

    // Initialize all lists of the group node (even if there are no shapes)
    returnGroup->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
    returnGroup->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
    returnGroup->paths = initializeList(&pathToString, &deletePath, &comparePaths);
    returnGroup->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
    returnGroup->otherAttributes = parseAttributes(groupNode);
    if (!(returnGroup->otherAttributes))
    {
        fprintf(stderr, "Group attributes parse failed\n");
        return NULL;
    }

    /*
    Using the same algorithm to traverse through the tree but isntead of an SVG
    node as parameter we use a xmlNode (representing a group). This is so we can
    use depth first search to also properly traverse and link the groups of groups/
    */
    for (currNode = groupNode->children; currNode != NULL; currNode = currNode->next)
    {
        char *name = (char *)currNode->name;

        if (strcasecmp(name, "g") == 0)
        {
            // Recursive call
            Group *currGroup = parseGroup(currNode);
            if (!currGroup)
            {
                fprintf(stderr, "Layer 1 Group Parse Failed\n");
                return NULL;
            }
            else
            {
                insertBack(returnGroup->groups, (void *)currGroup);
            }
        }

        else if (strcasecmp(name, "path") == 0)
        {
            Path *currPath = parsePath(currNode);
            if (!currPath)
            {
                fprintf(stderr, "Path parse failed\n");
                return NULL;
            }
            else
            {
                insertBack(returnGroup->paths, (void *)currPath);
            }
        }

        else if (strcasecmp(name, "circle") == 0)
        {
            Circle *currCircle = parseCircle(currNode);
            if (!currCircle)
            {
                fprintf(stderr, "Circle parse failed\n");
                return NULL;
            }
            else
            {
                insertBack(returnGroup->circles, (void *)currCircle);
            }
        }

        else if (strcasecmp(name, "rect") == 0)
        {
            Rectangle *currRect = parseRect(currNode);
            if (!currRect)
            {
                fprintf(stderr, "Rectangle parse failed\n");
                return NULL;
            }
            else
            {
                insertBack(returnGroup->rectangles, (void *)currRect);
            }
        }
    }

    // Returns a fully parsed Group node
    return returnGroup;
}

Path *parsePath(xmlNode *pathNode)
{
    Path *path;
    List *allAttributes;
    Attribute *dataAttrFinal;
    void *dataToDelete;
    int memLen = 0;

    allAttributes = parseAttributes(pathNode);
    if (!allAttributes)
    {
        fprintf(stderr, "Getting Path attributes failed\n");
        return NULL;
    }

    /*
    Since the data in a Path is a resizable array, we must get the
    size of the data from the first attribute element so that we can
    properly allocate space for the list
    */

    ListIterator attributeIteratorOne = createIterator(allAttributes);
    void *dataAttrVoid;

    while ((dataAttrVoid = nextElement(&attributeIteratorOne)) != NULL)
    {
        Attribute *tempCurrData = (Attribute *)dataAttrVoid;
        if (strcasecmp(tempCurrData->name, "d") == 0)
        {
            memLen = strlen(tempCurrData->value) + 1;
        }
    }

    if (memLen == 0)
    {
        fprintf(stderr, "Path data not found, NOT PATH\n");
        return NULL;
    }

    freeList(allAttributes);

    // Malloc given info
    path = (Path *)malloc(sizeof(Path) + (sizeof(char) * memLen));
    if (!path)
    {
        fprintf(stderr, "Path Node malloc failed\n");
        return NULL;
    }

    /*
    Since data is a resizeable is array we need to first
    store the data somewhere before we can remove it from attributes.
    This is because path->data must be the last thing to initialize.
    Delete old data, set other attributes then initialize path->data.
    */
    path->otherAttributes = parseAttributes(pathNode);
    ListIterator attributeIteratorTwo = createIterator(path->otherAttributes);
    char *temp = (char *)malloc(sizeof(char) * memLen);
    if (!temp)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }
    void *tempDataVar;

    while ((tempDataVar = nextElement(&attributeIteratorTwo)) != NULL)
    {
        dataAttrFinal = (Attribute *)tempDataVar;
        if (strcasecmp(dataAttrFinal->name, "d") == 0)
        {
            strcpy(temp, dataAttrFinal->value);

            dataToDelete = deleteDataFromList(path->otherAttributes, tempDataVar);
            if (dataToDelete != NULL)
            {
                path->otherAttributes->deleteData(dataToDelete);
            }
            else
            {
                fprintf(stderr, "Path Data Attribute not properly deleted parsePath\n");
                return NULL;
            }
            break;
        }
    }

    strcpy(path->data, temp);

    free(temp);

    return path;
}

Rectangle *parseRect(xmlNode *rectNode)
{
    Rectangle *rect = (Rectangle *)malloc(sizeof(Rectangle));
    ListIterator counter;
    void *currAttr;
    if (!rect)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    rect->otherAttributes = parseAttributes(rectNode);
    if (!(rect->otherAttributes))
    {
        fprintf(stderr, "Getting rectangle attributes failed\n");
        return NULL;
    }

    counter = createIterator(rect->otherAttributes);

    // UNITS MUST NOT BE NULL, <shape>->units[0] = '\0' will be used to represent a empty string
    rect->units[0] = '\0';

    /*
    Parse through all attributes, extract rectangle data and add to struct.
    All extracted data is deleted from list so that the final list will only
    be the other attributes left over
    */

    while ((currAttr = nextElement(&counter)))
    {
        Attribute *tempAttr = (Attribute *)currAttr;
        void *data;

        if (strcasecmp(tempAttr->name, "x") == 0)
        {
            extractUnits(&rect->units[0], tempAttr->value);
            rect->x = extractNumber(tempAttr->value);
            if (rect->x == -1)
            {
                fprintf(stderr, "Extract number for X in Rectangle struct failed\n");
                return NULL;
            }
            data = deleteDataFromList(rect->otherAttributes, tempAttr);
            if (data != NULL)
            {
                rect->otherAttributes->deleteData(data);
            }
            else
            {
                fprintf(stderr, "X data was not deleted properly from Rectangle attributes list\n");
            }
        }
        else if (strcasecmp(tempAttr->name, "y") == 0)
        {
            extractUnits(&rect->units[0], tempAttr->value);
            rect->y = extractNumber(tempAttr->value);
            if (rect->y == -1)
            {
                fprintf(stderr, "Extract number for Y in Rectangle struct failed\n");
                return NULL;
            }
            data = deleteDataFromList(rect->otherAttributes, tempAttr);
            if (data != NULL)
            {
                rect->otherAttributes->deleteData(data);
            }
            else
            {
                fprintf(stderr, "Y data was not deleted properly from Rectangle attributes list\n");
            }
        }
        else if (strcasecmp(tempAttr->name, "width") == 0)
        {
            extractUnits(&rect->units[0], tempAttr->value);
            rect->width = extractNumber(tempAttr->value);
            if (rect->width < 0)
            {
                fprintf(stderr, "Extract number for Width in Rectangle struct failed\n");
                return NULL;
            }
            data = deleteDataFromList(rect->otherAttributes, tempAttr);
            if (data != NULL)
            {
                rect->otherAttributes->deleteData(data);
            }
            else
            {
                fprintf(stderr, "Width data was not deleted properly from Rectangle attributes list\n");
            }
        }
        else if (strcasecmp(tempAttr->name, "height") == 0)
        {
            extractUnits(&rect->units[0], tempAttr->value);
            rect->height = extractNumber(tempAttr->value);
            if (rect->height < 0)
            {
                fprintf(stderr, "Extract number for Height in Rectangle struct failed\n");
                return NULL;
            }
            data = deleteDataFromList(rect->otherAttributes, tempAttr);
            if (data != NULL)
            {
                rect->otherAttributes->deleteData(data);
            }
            else
            {
                fprintf(stderr, "Height data was not deleted properly from Rectangle attributes list\n");
            }
        }
    }
    // Returns a fully parsed Rectangle node
    return rect;
}

Circle *parseCircle(xmlNode *circleNode)
{
    Circle *circle = (Circle *)malloc(sizeof(Circle));
    ListIterator counter;
    void *currAttr;

    if (!circle)
    {
        fprintf(stderr, "Malloc error occured\n");
        return NULL;
    }

    circle->otherAttributes = parseAttributes(circleNode);
    if (!(circle->otherAttributes))
    {
        fprintf(stderr, "Getting Circle attributes failed\n");
        return NULL;
    }

    counter = createIterator(circle->otherAttributes);

    // UNITS MUST NOT BE NULL, <shape>->units[0] = '\0' will be used to represent a empty string
    circle->units[0] = '\0';

    /*
    Parse through all attributes, extract circle data and add to struct.
    All extracted data is deleted from list so that the final list will only
    be the other attributes left over
    */

    while ((currAttr = nextElement(&counter)))
    {
        Attribute *tempAttr = (Attribute *)currAttr;
        void *data;

        if (strcasecmp(tempAttr->name, "cx") == 0)
        {
            extractUnits(&circle->units[0], tempAttr->value);
            circle->cx = extractNumber(tempAttr->value);
            if (circle->cx == -1)
            {
                fprintf(stderr, "Extract number for cX in Circle struct failed\n");
                return NULL;
            }
            data = deleteDataFromList(circle->otherAttributes, tempAttr);
            if (data != NULL)
            {
                circle->otherAttributes->deleteData(data);
            }
            else
            {
                fprintf(stderr, "cX data was not deleted properly from Circle attributes list\n");
            }
        }
        else if (strcasecmp(tempAttr->name, "cy") == 0)
        {
            extractUnits(&circle->units[0], tempAttr->value);
            circle->cy = extractNumber(tempAttr->value);
            if (circle->cy == -1)
            {
                fprintf(stderr, "Extract number for cY in Circle struct failed\n");
                return NULL;
            }
            data = deleteDataFromList(circle->otherAttributes, tempAttr);
            if (data != NULL)
            {
                circle->otherAttributes->deleteData(data);
            }
            else
            {
                fprintf(stderr, "cY data was not deleted properly from Circle attributes list\n");
            }
        }
        else if (strcasecmp(tempAttr->name, "r") == 0)
        {
            extractUnits(&circle->units[0], tempAttr->value);
            circle->r = extractNumber(tempAttr->value);
            if (circle->r < 0)
            {
                fprintf(stderr, "Extract number for Radius in circle struct failed\n");
                return NULL;
            }
            data = deleteDataFromList(circle->otherAttributes, tempAttr);
            if (data != NULL)
            {
                circle->otherAttributes->deleteData(data);
            }
            else
            {
                fprintf(stderr, "Radius data was not deleted properly from Circle attributes list\n");
            }
        }
    }

    // Returns a fully parsed Circle node
    return circle;
}

/*
ALL HELPER/LIST TRAVERSAL FUNCTIONS BELOW:
*/

float extractNumber(char *string)
{
    if (!string)
    {
        fprintf(stderr, "Cannot pass null string\n");
        return -1;
    }
    int memLen = strlen(string) + 1;
    char *str = (char *)malloc(sizeof(char) * memLen);
    if (!str)
    {
        fprintf(stderr, "Malloc error occured\n");
        return -1;
    }

    float returnFloat = 0.0;

    // Copy data from string, so that the original data is not changed
    strcpy(str, string);

    /*
    This for loop is derived from iabdalkader post on stackoverflow
    Link of author for this loop given in header file for this function.
    */

    for (; *str; str++)
    {
        if (!isalpha(*str))
        {
            returnFloat = strtof(str, NULL);
            break;
        }
    }

    if (returnFloat == 0.0)
    {
        return -1;
    }

    free(str);

    return returnFloat;
}

void extractUnits(char *units, char *string)
{
    int memLen = strlen(string) + 1;
    char allUnits[10][3] = {"cm", "in", "mm", "ft", "m", "em", "ex", "px", "pt", "pc"}; // LIST OF ALL COMMON MEASUREMENTS
    char str[memLen];

    // Copy data from string, so that the original data is not changed
    strcpy(str, string);

    /*
    The loop below checks if a substring (i.e measurement) is
    contained within the string we are checking. If it is we copy the measurement
    otherwise nothing will happen.
    */

    for (int i = 0; i < 10; i++)
    {
        if (strstr(str, allUnits[i]) != NULL)
        {
            strcpy(units, allUnits[i]);
            return;
        }
    }
}

void traverseGroups(List *currGroup, List *totalShapes, int elemType)
{
    if (!currGroup || !totalShapes)
    {
        fprintf(stderr, "Cannot pass NULL parameters\n");
        return;
    }

    List *elementList;
    ListIterator groupListLooper;
    groupListLooper = createIterator(currGroup);
    void *currAttr;

    /*
    The outer loop traverses through all groups in the group list passed,
    represented by currGroup. Depending on element type which is passed one
    it will then set the current element list to look at to that corresponding
    element list in the group struct. From there the inner loop will loop
    through the element list, adding each element node onto the totalShapes
    list which is passed from whichever function called it. It will check
    if the current group element contains another group within it, if so
    it will traverse all child loops using the same algorithm recursively/
    This will result in ordered lists that if toString was to be called on this
    totalShapes list, it will print all nodes of that element, in the order
    they appeared on the .SVG Document.
    */

    while ((currAttr = nextElement(&groupListLooper)))
    {
        Group *tempGroupNode = (Group *)currAttr;
        if (elemType == CIRC)
        {
            elementList = tempGroupNode->circles;
        }
        else if (elemType == RECT)
        {
            elementList = tempGroupNode->rectangles;
        }
        else if (elemType == PATH)
        {
            elementList = tempGroupNode->paths;
        }
        else if (elemType == GROUP)
        {
            elementList = tempGroupNode->groups;
        }

        ListIterator elementListIterator;
        elementListIterator = createIterator(elementList);
        void *elemCurrNode;

        while ((elemCurrNode = nextElement(&elementListIterator)))
        {
            insertBack(totalShapes, elemCurrNode);
        }

        if (getLength(tempGroupNode->groups) > 0)
        {
            traverseGroups(tempGroupNode->groups, totalShapes, elemType);
        }
    }
}

int traverseWholeSVG(List *currShape, int elemType)
{
    int counter = 0;
    ListIterator listLooper;
    listLooper = createIterator(currShape);
    void *currAttr;

    /*
    Depending on which element type we are looking at, the code below
    will traverse through the other attributes of each element within
    the shape list. While traversing each element it will get the length
    of each otherAttributes list for each element and finally the function
    returns the number of ALL other attributes within the given element
    List.
    */

    if (elemType == CIRC)
    {
        while ((currAttr = nextElement(&listLooper)))
        {
            Circle *circleNode = (Circle *)currAttr;
            counter += getLength(circleNode->otherAttributes);
        }
    }
    else if (elemType == RECT)
    {
        while ((currAttr = nextElement(&listLooper)))
        {
            Rectangle *rectNode = (Rectangle *)currAttr;
            counter += getLength(rectNode->otherAttributes);
        }
    }
    else if (elemType == PATH)
    {
        while ((currAttr = nextElement(&listLooper)))
        {
            Path *pathNode = (Path *)currAttr;
            counter += getLength(pathNode->otherAttributes);
        }
    }
    else if (elemType == GROUP)
    {
        while ((currAttr = nextElement(&listLooper)))
        {
            Group *groupNode = (Group *)currAttr;
            counter += getLength(groupNode->otherAttributes);
        }
    }

    return counter;
}

void dummyDeleteFunc(void *dummy)
{
}

/*
ALL ASSIGNMENT 2 HELPER FUNCTIONS BELOW:
*/

xmlDoc *SVGtoXmlDoc(const SVG *img)
{
    if (!img || !(img->rectangles) || !(img->circles) || !(img->paths) || !(img->groups) || !(img->otherAttributes) || strlen(img->namespace) <= 0)
    {
        fprintf(stderr, "Invalid parameter SVGtoXmlDoc\n");
        return NULL;
    }

    xmlDoc *doc = NULL;
    xmlNode *root_node = NULL;
    xmlNs *currDocNS = NULL;
    ListIterator counter;
    ListIterator otherAttributes;
    void *data;
    void *attrData;

    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewDocNode(doc, NULL, BAD_CAST "svg", NULL);
    currDocNS = xmlNewNs(root_node, BAD_CAST img->namespace, NULL);
    xmlSetNs(root_node, currDocNS);
    xmlDocSetRootElement(doc, root_node);

    if (strlen(img->title) >= 0)
    {
        xmlNewChild(root_node, NULL, BAD_CAST "title", BAD_CAST img->title);
    }

    if (strlen(img->description) >= 0)
    {
        xmlNewChild(root_node, NULL, BAD_CAST "desc", BAD_CAST img->description);
    }

    /*
    Rectangles
    */
    counter = createIterator(img->rectangles);
    while ((data = nextElement(&counter)) != NULL)
    {
        xmlNode *currNode;
        Rectangle *rect = (Rectangle *)data;
        char tempBuffer[200];

        currNode = xmlNewChild(root_node, NULL, BAD_CAST "rect", NULL);

        if (strlen(rect->units) > 0)
        {
            sprintf(tempBuffer, "%f%s", rect->x, rect->units);
            xmlNewProp(currNode, BAD_CAST "x", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", rect->y, rect->units);
            xmlNewProp(currNode, BAD_CAST "y", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", rect->width, rect->units);
            xmlNewProp(currNode, BAD_CAST "width", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", rect->height, rect->units);
            xmlNewProp(currNode, BAD_CAST "height", BAD_CAST tempBuffer);
        }
        else
        {
            sprintf(tempBuffer, "%f", rect->x);
            xmlNewProp(currNode, BAD_CAST "x", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", rect->y);
            xmlNewProp(currNode, BAD_CAST "y", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", rect->width);
            xmlNewProp(currNode, BAD_CAST "width", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", rect->height);
            xmlNewProp(currNode, BAD_CAST "height", BAD_CAST tempBuffer);
        }

        if (!(rect->otherAttributes))
        {
            fprintf(stderr, "Rectangle other attributes CANNOT be NULL SVGtoXmlDoc\n");
            return NULL;
        }
        otherAttributes = createIterator(rect->otherAttributes);
        while ((attrData = nextElement(&otherAttributes)) != NULL)
        {
            Attribute *attribute = (Attribute *)attrData;
            xmlNewProp(currNode, BAD_CAST attribute->name, BAD_CAST attribute->value);
        }
    }

    /*
    Circles
    */
    counter = createIterator(img->circles);
    while ((data = nextElement(&counter)) != NULL)
    {
        xmlNode *currNode;
        Circle *circ = (Circle *)data;
        char tempBuffer[200];

        currNode = xmlNewChild(root_node, NULL, BAD_CAST "circle", NULL);

        if (strlen(circ->units) > 0)
        {
            sprintf(tempBuffer, "%f%s", circ->cx, circ->units);
            xmlNewProp(currNode, BAD_CAST "cx", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", circ->cy, circ->units);
            xmlNewProp(currNode, BAD_CAST "cy", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", circ->r, circ->units);
            xmlNewProp(currNode, BAD_CAST "r", BAD_CAST tempBuffer);
        }
        else
        {
            sprintf(tempBuffer, "%f", circ->cx);
            xmlNewProp(currNode, BAD_CAST "cx", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", circ->cy);
            xmlNewProp(currNode, BAD_CAST "cy", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", circ->r);
            xmlNewProp(currNode, BAD_CAST "r", BAD_CAST tempBuffer);
        }

        if (!(circ->otherAttributes))
        {
            fprintf(stderr, "Circles other attributes CANNOT be NULL SVGtoXmlDoc\n");
            return NULL;
        }
        otherAttributes = createIterator(circ->otherAttributes);
        while ((attrData = nextElement(&otherAttributes)) != NULL)
        {
            Attribute *attribute = (Attribute *)attrData;
            xmlNewProp(currNode, BAD_CAST attribute->name, BAD_CAST attribute->value);
        }
    }

    /*
    Paths
    */
    counter = createIterator(img->paths);
    while ((data = nextElement(&counter)) != NULL)
    {
        xmlNode *currNode;
        Path *currPath = (Path *)data;

        currNode = xmlNewChild(root_node, NULL, BAD_CAST "path", NULL);
        xmlNewProp(currNode, BAD_CAST "d", BAD_CAST currPath->data);

        if (!(currPath->otherAttributes))
        {
            fprintf(stderr, "Paths other attributes CANNOT be NULL SVGtoXmlDoc\n");
            return NULL;
        }
        otherAttributes = createIterator(currPath->otherAttributes);
        while ((attrData = nextElement(&otherAttributes)) != NULL)
        {
            Attribute *attribute = (Attribute *)attrData;
            xmlNewProp(currNode, BAD_CAST attribute->name, BAD_CAST attribute->value);
        }
    }

    /*
    Groups
    */
    counter = createIterator(img->groups);
    while ((data = nextElement(&counter)) != NULL)
    {
        xmlNode *currNode;
        Group *currGroup = (Group *)data;

        currNode = xmlNewChild(root_node, NULL, BAD_CAST "g", NULL);
        if (addGroupsToDoc(currNode, currGroup) == -1)
        {
            fprintf(stderr, "Error occurred adding groups\n");
            return NULL;
        }
    }

    /*
    Other Attributes
    */
    otherAttributes = createIterator(img->otherAttributes);
    while ((attrData = nextElement(&otherAttributes)) != NULL)
    {
        Attribute *attribute = (Attribute *)attrData;
        xmlNewProp(root_node, BAD_CAST attribute->name, BAD_CAST attribute->value);
    }

    xmlCleanupParser();

    return doc;
}

int addGroupsToDoc(xmlNode *currRoot, Group *parentGroup)
{
    if (!(parentGroup->rectangles) || !(parentGroup->circles) || !(parentGroup->paths) || !(parentGroup->groups) || !(parentGroup->otherAttributes))
    {
        fprintf(stderr, "Group Lists CANNOT be NULL addGroupsToDoc\n");
        return -1;
    }

    ListIterator counter;
    ListIterator otherAttributes;
    char *data;
    char *attrData;

    // Other Attributes of Group
    counter = createIterator(parentGroup->otherAttributes);
    while ((data = nextElement(&counter)) != NULL)
    {
        Attribute *attribute = (Attribute *)data;
        xmlNewProp(currRoot, BAD_CAST attribute->name, BAD_CAST attribute->value);
    }

    /*
     Rectangles
     */
    counter = createIterator(parentGroup->rectangles);
    while ((data = nextElement(&counter)) != NULL)
    {
        xmlNode *currNode;
        Rectangle *rect = (Rectangle *)data;
        char tempBuffer[200];

        currNode = xmlNewChild(currRoot, NULL, BAD_CAST "rect", NULL);

        if (strlen(rect->units) > 0)
        {
            sprintf(tempBuffer, "%f%s", rect->x, rect->units);
            xmlNewProp(currNode, BAD_CAST "x", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", rect->y, rect->units);
            xmlNewProp(currNode, BAD_CAST "y", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", rect->width, rect->units);
            xmlNewProp(currNode, BAD_CAST "width", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", rect->height, rect->units);
            xmlNewProp(currNode, BAD_CAST "height", BAD_CAST tempBuffer);
        }
        else
        {
            sprintf(tempBuffer, "%f", rect->x);
            xmlNewProp(currNode, BAD_CAST "x", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", rect->y);
            xmlNewProp(currNode, BAD_CAST "y", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", rect->width);
            xmlNewProp(currNode, BAD_CAST "width", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", rect->height);
            xmlNewProp(currNode, BAD_CAST "height", BAD_CAST tempBuffer);
        }

        if (!(rect->otherAttributes))
        {
            fprintf(stderr, "Rectangle other attributes CANNOT be NULL SVGtoXmlDoc\n");
            return -1;
        }
        otherAttributes = createIterator(rect->otherAttributes);
        while ((attrData = nextElement(&otherAttributes)) != NULL)
        {
            Attribute *attribute = (Attribute *)attrData;
            xmlNewProp(currNode, BAD_CAST attribute->name, BAD_CAST attribute->value);
        }
    }

    /*
    Circles
    */
    counter = createIterator(parentGroup->circles);
    while ((data = nextElement(&counter)) != NULL)
    {
        xmlNode *currNode;
        Circle *circ = (Circle *)data;
        char tempBuffer[200];

        currNode = xmlNewChild(currRoot, NULL, BAD_CAST "circle", NULL);

        if (strlen(circ->units) > 0)
        {
            sprintf(tempBuffer, "%f%s", circ->cx, circ->units);
            xmlNewProp(currNode, BAD_CAST "cx", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", circ->cy, circ->units);
            xmlNewProp(currNode, BAD_CAST "cy", BAD_CAST tempBuffer);

            sprintf(tempBuffer, "%f%s", circ->r, circ->units);
            xmlNewProp(currNode, BAD_CAST "r", BAD_CAST tempBuffer);
        }
        else
        {
            sprintf(tempBuffer, "%f", circ->cx);
            xmlNewProp(currNode, BAD_CAST "cx", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", circ->cy);
            xmlNewProp(currNode, BAD_CAST "cy", BAD_CAST tempBuffer);
            sprintf(tempBuffer, "%f", circ->r);
            xmlNewProp(currNode, BAD_CAST "r", BAD_CAST tempBuffer);
        }

        if (!(circ->otherAttributes))
        {
            fprintf(stderr, "Circles other attributes CANNOT be NULL SVGtoXmlDoc\n");
            return -1;
        }
        otherAttributes = createIterator(circ->otherAttributes);
        while ((attrData = nextElement(&otherAttributes)) != NULL)
        {
            Attribute *attribute = (Attribute *)attrData;
            xmlNewProp(currNode, BAD_CAST attribute->name, BAD_CAST attribute->value);
        }
    }

    /*
    Paths
    */
    counter = createIterator(parentGroup->paths);
    while ((data = nextElement(&counter)) != NULL)
    {
        xmlNode *currNode;
        Path *currPath = (Path *)data;

        currNode = xmlNewChild(currRoot, NULL, BAD_CAST "path", NULL);
        xmlNewProp(currNode, BAD_CAST "d", BAD_CAST currPath->data);

        if (!(currPath->otherAttributes))
        {
            fprintf(stderr, "Paths other attributes CANNOT be NULL SVGtoXmlDoc\n");
            return -1;
        }
        otherAttributes = createIterator(currPath->otherAttributes);
        while ((attrData = nextElement(&otherAttributes)) != NULL)
        {
            Attribute *attribute = (Attribute *)attrData;
            xmlNewProp(currNode, BAD_CAST attribute->name, BAD_CAST attribute->value);
        }
    }

    // Groups of Group
    counter = createIterator(parentGroup->groups);
    while ((data = nextElement(&counter)) != NULL)
    {
        xmlNode *currNode;
        Group *currGroup = (Group *)data;

        currNode = xmlNewChild(currRoot, NULL, BAD_CAST "g", NULL);
        if (addGroupsToDoc(currNode, currGroup) == -1)
        {
            fprintf(stderr, "Error occurred adding groups\n");
            return -1;
        }
    }

    xmlCleanupParser();

    return 1;
}

bool validXmlTree(xmlDoc *doc, const char *schemaFile)
{
    if (!doc || !schemaFile || strlen(schemaFile) <= 0)
    {
        fprintf(stderr, "Invalid parameters passed validXmlTree\n");
        return false;
    }

    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxtTemp;

    xmlLineNumbersDefault(1);

    ctxtTemp = xmlSchemaNewParserCtxt(schemaFile);
    xmlSchemaSetParserErrors(ctxtTemp, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
    schema = xmlSchemaParse(ctxtTemp);
    xmlSchemaFreeParserCtxt(ctxtTemp);

    xmlSchemaValidCtxtPtr ctxtMain;
    int ret;

    ctxtMain = xmlSchemaNewValidCtxt(schema);
    xmlSchemaSetValidErrors(ctxtMain, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
    ret = xmlSchemaValidateDoc(ctxtMain, doc);

    xmlSchemaFreeValidCtxt(ctxtMain);
    if (schema != NULL)
    {
        xmlSchemaFree(schema);
        xmlSchemaCleanupTypes();
    }
    xmlMemoryDump();

    if (ret == 0)
    {
        return true;
    }

    return false;
}

bool validateAttributes(List *attributeList)
{
    if (!attributeList)
    {
        fprintf(stderr, "cannot pass NULL attributes list validateAttributes\n");
        return false;
    }
    ListIterator count = createIterator(attributeList);
    Attribute *data;

    while ((data = (Attribute *)nextElement(&count)) != NULL)
    {
        if (!(data->name) || !(data->value))
        {
            return false;
        }
    }

    return true;
}

bool validateAllGroups(List *groupList)
{
    if (!groupList)
    {
        fprintf(stderr, "cannot pass null group list validateAllGroups\n");
        return false;
    }
    ListIterator counter = createIterator(groupList);
    Group *data;

    while ((data = (Group *)nextElement(&counter)) != NULL)
    {
        if (!(validateAllGroups(data->groups)) || !(validateCircles(data->circles)) || !(validatePaths(data->paths)) || !(validateRects(data->rectangles)) || !(validateAttributes(data->otherAttributes)))
        {
            return false;
        }
    }

    return true;
}

bool validateRects(List *rectList)
{
    if (!rectList)
    {
        fprintf(stderr, "cannot pass null rect list validateRects\n");
        return false;
    }

    ListIterator counter = createIterator(rectList);
    Rectangle *data;

    while ((data = (Rectangle *)nextElement(&counter)) != NULL)
    {
        if (data->width < 0 || data->height < 0)
        {
            return false;
        }

        if (!(validateAttributes(data->otherAttributes)))
        {
            return false;
        }
    }

    return true;
}

bool validateCircles(List *circleList)
{
    if (!circleList)
    {
        fprintf(stderr, "cannot pass null circle list validateCircles\n");
        return false;
    }
    ListIterator counter = createIterator(circleList);
    Circle *data;

    while ((data = (Circle *)nextElement(&counter)) != NULL)
    {
        if (data->r < 0)
        {
            return false;
        }
        if (!(validateAttributes(data->otherAttributes)))
        {
            return false;
        }
    }

    return true;
}

bool validatePaths(List *pathList)
{
    if (!pathList)
    {
        fprintf(stderr, "cannot pass null path list, validatePaths\n");
        return false;
    }

    ListIterator counter = createIterator(pathList);
    Path *data;

    while ((data = (Path *)nextElement(&counter)) != NULL)
    {
        if (!(data->data))
        {
            return false;
        }
        if (!(validateAttributes(data->otherAttributes)))
        {
            return false;
        }
    }

    return true;
}

int setOtherAttr(List *otherAttributes, Attribute *attr)
{
    if (!otherAttributes || !attr)
    {
        fprintf(stderr, "Invalid params setOtherAttr\n");
        return -1;
    }
    if (!(attr->name) || strlen(attr->value) <= 0)
    {
        fprintf(stderr, "Invalid params setOtherAttr\n");
        return -1;
    }

    Node *currHead = otherAttributes->head;
    while (currHead)
    {
        if (strcasecmp(((Attribute *)currHead->data)->name, attr->name) == 0)
        {
            if (!(attr->value))
            {
                fprintf(stderr, "Given attribute value is NULL\n");
                return -1;
            }
            int valueMemLen = strlen(attr->value) + 1;
            int nameMemLen = strlen(attr->name) + 1;
            currHead->data = realloc(currHead->data, sizeof(Attribute) + (sizeof(char) * valueMemLen));
            ((Attribute *)currHead->data)->name = realloc(((Attribute *)currHead->data)->name, sizeof(char) * nameMemLen);
            strcpy(((Attribute *)currHead->data)->name, attr->name);
            strcpy(((Attribute *)currHead->data)->value, attr->value);
            return 1;
        }
        currHead = currHead->next;
    }
    insertBack(otherAttributes, (void *)attr);
    return 2;
}

int setCircAttr(void *data, Attribute *attr)
{
    if (!data || !attr || !(attr->name) || strlen(attr->value) <= 0)
    {
        fprintf(stderr, "Invalid params setCircleAttr\n");
        return -1;
    }

    Circle *circleElement = (Circle *)data;
    float temp;

    if (strcasecmp(attr->name, "cx") == 0)
    {
        temp = atof(attr->value);
        circleElement->cx = temp;
    }

    else if (strcasecmp(attr->name, "cy") == 0)
    {
        temp = atof(attr->value);
        circleElement->cy = temp;
    }

    else if (strcasecmp(attr->name, "r") == 0)
    {
        temp = atof(attr->value);
        if (temp < 0)
        {
            fprintf(stderr, "cannot set radius under 0\n");
            return -1;
        }
        circleElement->r = temp;
    }

    else if (strcasecmp(attr->name, "units") == 0)
    {
        if (strlen(attr->value) >= 50)
        {
            strncpy(circleElement->units, attr->value, 50);
        }
        else if (strlen(attr->value) >= 0)
        {
            strcpy(circleElement->units, attr->value);
        }
    }

    else
    {
        if (!(circleElement->otherAttributes))
        {
            fprintf(stderr, "invalid circle other attributes setCircAttr\n");
            return -1;
        }
        return setOtherAttr(circleElement->otherAttributes, attr);
    }

    return 1;
}

int setRectAttr(void *data, Attribute *attr)
{
    if (!data || !attr || !(attr->name) || strlen(attr->value) <= 0)
    {
        fprintf(stderr, "Invalid params setRectAttr\n");
        return -1;
    }

    Rectangle *rectElement = (Rectangle *)data;
    float temp;

    if (strcasecmp(attr->name, "x") == 0)
    {
        temp = atof(attr->value);
        rectElement->x = temp;
    }

    else if (strcasecmp(attr->name, "y") == 0)
    {
        temp = atof(attr->value);
        rectElement->y = temp;
    }

    else if (strcasecmp(attr->name, "width") == 0)
    {
        temp = atof(attr->value);
        if (temp < 0)
        {
            fprintf(stderr, "cannot set rectangle width under 0\n");
            return -1;
        }
        rectElement->width = temp;
    }

    else if (strcasecmp(attr->name, "height") == 0)
    {
        temp = atof(attr->value);
        if (temp < 0)
        {
            fprintf(stderr, "cannot set rectangle height under 0\n");
            return -1;
        }
        rectElement->height = temp;
    }

    else if (strcasecmp(attr->name, "units") == 0)
    {
        if (strlen(attr->value) >= 50)
        {
            strncpy(rectElement->units, attr->value, 50);
        }
        else if (strlen(attr->value) >= 0)
        {
            strcpy(rectElement->units, attr->value);
        }
    }

    else
    {
        if (!(rectElement->otherAttributes))
        {
            fprintf(stderr, "invalid rectangle other attributes setRectAttr\n");
            return -1;
        }
        return setOtherAttr(rectElement->otherAttributes, attr);
    }

    return 1;
}

int setPathAttr(List *pathList, Attribute *attr, int index)
{
    if (!pathList || !attr || !(attr->name) || !(attr->value) || strlen(attr->value) <= 0)
    {
        fprintf(stderr, "Invalid params setPathAttr\n");
        return -1;
    }

    Node *currHead = pathList->head;
    int count = 0;

    while (currHead)
    {
        if (count == index)
        {
            if (strcasecmp(attr->name, "d") == 0)
            {
                if (!(((Path *)currHead->data)->data))
                {
                    fprintf(stderr, "Path at given index is NULL\n");
                    return -1;
                }
                int memLen = strlen(attr->value) + 1;
                currHead->data = realloc(currHead->data, sizeof(Path) + (sizeof(char) * memLen));
                strcpy(((Path *)currHead->data)->data, attr->value);
                break;
            }
            else
            {
                if (!(((Path *)currHead->data)->otherAttributes))
                {
                    fprintf(stderr, "invalid path other attributes setPathAttr\n");
                    return -1;
                }
                return setOtherAttr(((Path *)currHead->data)->otherAttributes, attr);
            }
        }
        currHead = currHead->next;
        count++;
    }

    return 1;
}

int setGroupAttr(void *data, Attribute *attr)
{
    if (!data || !attr || !(attr->name) || strlen(attr->value) <= 0)
    {
        fprintf(stderr, "Invalid params setGroupAttr\n");
        return -1;
    }

    Group *groupElement = (Group *)data;

    if (!(groupElement->otherAttributes))
    {
        fprintf(stderr, "invalid group other attributes setGroupAttr\n");
        return -1;
    }

    return setOtherAttr(groupElement->otherAttributes, attr);
}

bool validateSVGStruct(const SVG *img)
{
    if (!img || strlen(img->namespace) <= 0 || !(validateAllGroups(img->groups)) || !(validateCircles(img->circles)) || !(validatePaths(img->paths)) || !(validateRects(img->rectangles)) || !(validateAttributes(img->otherAttributes)))
    {
        return false;
    }
    return true;
}

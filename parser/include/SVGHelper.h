/**
 * @file SVGHelper.h
 * @author Usman Zaheer (Student# 1148583)
 * @date February 2021
 * @brief File containing the function definitions of functions in SVGParser.c
 */

#include <math.h>
#include <ctype.h>
#include <strings.h>
#include "SVGParser.h"

/** Function to initialize/parse a SVG struct while traversing the xmlDoc tree starting
 * at the xmlNode given.
 * @pre Pointer arguments must not be NULL.
 * @post The SVG Node pointer will have been fully initialized with every element/attribute from the xmlDoc tree starting at root.
 * @return On a successful tree traversal parse, it will return 0. Any failures and it will return -1.
 * @param SVGNode - pointer to a already declared (malloc and Lists initialized) SVG Struct.
 * @param rootElement - pointer to the root element of an xml Document (from xmlDoc*).
 **/
int parseTree(SVG *SVGNode, xmlNode *root);

/** Function to initialize a List of ALL attributes from a given xmlNode.
 * @pre Pointer arguments must not be NULL.
 * @post Initialized List containing all attibutes from given xmlNode.
 * @return On a successful xmlNode parsing, it will return an initialized List. Any failures and it will return NULL.
 * @param node - pointer to a xmlNode from the xmlTree.
 **/
List *parseAttributes(xmlNode *node);

/** Function to allocate and initialize a Group struct pointer from a xmlNode.
 * @pre Pointer arguments must not be NULL.
 * @post Initialized Group pointer containing all data within that group xmlNode.
 * @return On a successful xmlNode parsing, it will return an initialized pointer
 * to a group struct. Any failures and it will return NULL.
 * @param node - pointer to a xmlNode from the xmlTree
 **/
Group *parseGroup(xmlNode *groupNode);

/** Function to allocate and initialize a Path struct pointer from a xmlNode.
 * @pre Pointer arguments must not be NULL.
 * @post Initialized Path pointer containing all data within that path xmlNode.
 * @return On a successful xmlNode parsing, it will return an initialized pointer
 * to a Path struct. Any failures and it will return NULL.
 * @param node - pointer to a xmlNode from the xmlTree
 **/
Path *parsePath(xmlNode *pathNode);

/** Function to allocate and initialize a Rectangle struct pointer from a xmlNode.
 * @pre Pointer arguments must not be NULL.
 * @post Initialized Rectangle pointer containing all data within that Rectangle xmlNode.
 * @return On a successful xmlNode parsing, it will return an initialized pointer
 * to a Rectangle struct. Any failures and it will return NULL.
 * @param node - pointer to a xmlNode from the xmlTree
 **/
Rectangle *parseRect(xmlNode *rectNode);

/** Function to allocate and initialize a Circle struct pointer from a xmlNode.
 * @pre Pointer arguments must not be NULL.
 * @post Initialized Circle pointer containing all data within that Circle xmlNode.
 * @return On a successful xmlNode parsing, it will return an initialized pointer
 * to a Circle struct. Any failures and it will return NULL.
 * @param node - pointer to a xmlNode from the xmlTree
 **/
Circle *parseCircle(xmlNode *cicleNode);

/** Function to extract all numbers from a given string in the float format.
 * @pre Pointer arguments must not be NULL.
 * @post Returns the extracted float from the string parameter.
 * @return On a successful parse, a float containg the numbers within the
 * string parameter will be returned. Any failure will return -1
 * @param string - Any valid string
 **/
float extractNumber(char *string);

/** Function to find measurement unit from any valid string. Example "Mg+'}46:.cm"
 * cm will be extracted and stored in the units parameter. The for loop
 * inside this function was derived driectly from a for loop I found on stackover
 * link is https://stackoverflow.com/questions/13698449/extract-double-from-a-string-in-c
 * and the author of the for loop is iabdalkader. The reason strtof or atof was not used
 * is to account for the cases where there maybe letters in front of the numbers.
 * @pre Pointer arguments must not be NULL.
 * @post On a successful parse, the units parameter will be initialized
 * with whatever valid unit of measurement was contained in the string.
 * If parsing was unsuccessfull or nothing was found, units will be
 * returned as it was passed.
 * @param units - A pointer to the string (&char[0]) to hold data type
 * @param string - Any valid string
 **/
void extractUnits(char *units, char *string);

/** This function will recursively search through all groups and any nested groups as
 * well. It will then populate the given totalShapes struct with elements of the
 * element type using for. Refer to ENUM to see element types and their corresponding
 * integers
 * @pre Pointer arguments must not be NULL. Integer argument must be between
 * either 1, 2, 3 or 4.
 * @post On traversal of all groups (nested as well) it will populate
 * total Shapes struct with whatever elemType integer we pass it.
 * @param currGroup - A pointer to the list of groups
 * @param totalShapes - A List of any elements defined in SVG
 * @param elemType - An integer represented by the ENUM in SVGParser.h
 **/
void traverseGroups(List *currGroup, List *totalShapes, int elemType);

/** This function will fully traverse the other attributes of each element
 * in the given ELEMENT list and return the number of OTHER attributes.
 * @pre Pointer arguments must not be NULL. Integer argument must be between
 * either 1, 2, 3 or 4.
 * @return On successful traversal it will return the total OTHER attributes
 * of each element within the passed list
 * @param currShape - A pointer to the list of type Elements
 * @param elemType - An integer represented by the ENUM in SVGParser.h
 **/
int traverseWholeSVG(List *currShape, int elemType);

/** Simple dummy delete function to create a secondary List of any element
 * This is to prevent deletion of the actual elements data when get<Shape> is
 * called.
 * @param dumm - Dummy variable, it is not mutated in anyway.
 **/
void dummyDeleteFunc(void *dummy);

/** This function takes a non NULL svg struct and converts it to
 * and xmlDoc and returns that xmlDoc.
 * @pre Pointer arguments must not be NULL
 * @post This function will not free the SVG img, caller must do that.
 * @return On successful traversal it will return a valid xmlDoc struct pointer
 * @param img - A pointer to the SVG struct
 **/
xmlDoc *SVGtoXmlDoc(const SVG *img);

/** This function implements the SVGtoXmlDoc algorithm and applies it.
 * Recursively for to account for nested groups within the Struct.
 * @pre Pointer arguments must not be NULL
 * @return On successful group traversal it will return 1, any errors will return -1.
 * @param currRoot - A pointer to the xmlNode that the group must be attached too
 * @param parentGroup - The group Struct itself which will be convertet to an xmlNode
 **/
int addGroupsToDoc(xmlNode *currRoot, Group *parentGroup);

/** This function will validate if the xmlTree is a valid in accordance to given schemaFile.
 * schemaFile must exist and be valid
 * @pre Pointer arguments must not be NULL
 * @post This function will not close/free the xmlDoc or schemaFile string, caller is responsible.
 * @return If the xmlTree follows the constraints specified by the schemaFile it will return true.
 * If it is not valid or any errors occur false will be returned.
 * @param fileName - Pointer to xmlTree
 * @param schemaFile - Pointer to the schema file name
 **/
bool validXmlTree(xmlDoc *doc, const char *schemaFile);

/** The following validate<Shape> traverses a List of a given shape and verifies if
 * it follows all SVG constraints as per SVGParser.h
 * @pre List pointer arguments not be NULL
 * @return If the shape lists follow all constraints the function will return true.
 * Any errors or invalid arguments will result in false return.
 * @param List - Pointer to List containing certain shape type
 **/
bool validateAttributes(List *attributeList);
bool validateAllGroups(List *groupList);
bool validateRects(List *rectList);
bool validateCircles(List *circleList);
bool validatePaths(List *pathList);

/** The following set<Shape>Attr will take a user passed Attribute and either
 * update or append (depends on if it exists or not) it to the passed shape.
 * @pre Pointer arguments must not be NULL and follow SVGParser.h constraints
 * @return If appeneded/updated successfully, function will return 1. Any
 * errors will result in -1 return.
 * @param data - Pointer to shape pointer casted as void *.
 * @param attr - User given attribute.
 **/
int setCircAttr(void *data, Attribute *attr);
int setRectAttr(void *data, Attribute *attr);
int setGroupAttr(void *data, Attribute *attr);

/**
 * @pre Pointer arguments must not be NULL and follow SVGParser.h constraints
 * @return If appeneded/updated successfully, function will return 1. Any
 * errors will result in -1 return.
 * @param otherAttributes - List containing otherAttributes of any given struct.
 * @param attr - User given attribute.
 **/
int setOtherAttr(List *otherAttributes, Attribute *attr);

/** This functions follows a serpeate format due to the nature of reallocating memory for
 * flexible array members in C. To account for the weird issue instead of traversing the
 * List with the given List API iterator, we do it manually to prevent errors and properly
 * reallocate memory in the case the attribute value passed is greater than the memory
 * that was initially allocated for that given resizeable array.
 * @pre Pointer arguments must not be NULL and follow SVGParser.h constraints
 * @return If appeneded/updated successfully, function will return 1. Any
 * errors will result in -1 return.
 * @param pathList - List containing paths. The path to be changed must be in list.
 * @param attr - User given attribute.
 * @param index - index of which path the user/caller wants to update.
 **/
int setPathAttr(List *pathList, Attribute *attr, int index);

/** The following function will traverse the whole SVG structure and validate.
 * All data in the struct making sure no NULL values/Lists are stored and
 * all filled attribute fields account for constraints given by SVGParser.h
 * @return If the passed SVG struct is valid, true will be returned.
 * If struct is invalid in any way -1 will be returned.
 * @param img - Pointer to SVG struct.
 **/
bool validateSVGStruct(const SVG *img);

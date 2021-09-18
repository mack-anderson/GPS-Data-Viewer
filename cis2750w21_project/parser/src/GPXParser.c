/*Author: Mack Anderson*/
/*StudentID: 1099411*/
/*Some portions of code are not my own and were taken from the in class example file libXmlExample.c and lecture 6*/
/*portions of http://www.xmlsoft.org/examples/tree2.c were used to help convert gpxDoc to xmlTree*/
/*portions of http://www.xmlsoft.org/examples/tree2.c were used to save xmlTree to a file*/
/*portions of http://knol2share.blogspot.com/2009/05/validate-xml-against-xsd-in-c.html were used to validate an xmlTree*/
/*portions of https://www.movable-type.co.uk/scripts/latlong.html were used to calculate distances between waypoints using the Haversine formula*/
#include "GPXParser.h"

/* create functions ------------------------------------------------------------------------------- */
GPXData* createGPXData(char *name, char *value){
    /* malloc gpxData struct */
    GPXData *gpxData = malloc(sizeof(GPXData) + sizeof(char)*(strlen((char*)value) + 1));

    /*check for successful malloc */
    if(gpxData == NULL){
        return NULL;
    }

    /* copy data to struct */
    strcpy(gpxData->name, name);
    strcpy(gpxData->value, value);
    
    return gpxData;
}

Waypoint* createWaypoint(char *name, double longitude, double latitude){
    /* allocate memory for Waypoint */
    Waypoint *waypoint = malloc(sizeof(Waypoint));
    if(waypoint == NULL){
        return NULL;
    }

    /* allocate memory for string */
    char* tempName = malloc(sizeof(char) * (strlen(name) + 1));
    if(tempName == NULL){
        return NULL;
    }
    strcpy(tempName, name);

    /* copy data to struct */
    waypoint->name = tempName;
    waypoint->longitude = longitude;
    waypoint->latitude = latitude;
    waypoint->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    return waypoint;
}

Route* createRoute(char* name){
    /* allocate memory for Route */
    Route *route = malloc(sizeof(Route));
    if(route == NULL){
        return NULL;
    }

    /* allocate memory for string */
    char* tempName = malloc((sizeof(char) * (strlen(name)) + 1));
    if(tempName == NULL){
        return NULL;
    }
    strcpy(tempName, name);

    route->name = tempName;
    route->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    route->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    return route;
}

TrackSegment* createTrackSegment(){
    TrackSegment *trackSegment = malloc(sizeof(TrackSegment));
    if(trackSegment == NULL){
        return NULL;
    }
    trackSegment->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    return trackSegment;
}

Track* createTrack(char* name){
    Track *track = malloc(sizeof(Track));
    if(track == NULL){
        return NULL;
    }
    char *temp = malloc(sizeof(char)*strlen(name)+1);
    if(temp == NULL){
        return NULL;
    }
    track->name = temp;
    track->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTracks);
    track->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    return track;
}

void setName(xmlNode * cur_node, GPXdoc *gpxDoc){
    char *parentName = (char*)((cur_node->parent)->parent)->name;
    if (strcmp(parentName, "wpt") == 0)
    {
        Waypoint *waypoint = getFromBack(gpxDoc->waypoints);
        free(waypoint->name);
        waypoint->name = (char*)xmlNodeGetContent(cur_node);
    }
    else if (strcmp(parentName, "rte") == 0)
    {
        Route *route = getFromBack(gpxDoc->routes);
        free(route->name);
        route->name = (char*)xmlNodeGetContent(cur_node);
    }
    else if (strcmp(parentName, "rtept") == 0)
    {
        Route *route = getFromBack(gpxDoc->routes);
        Waypoint *waypoint = getFromBack(route->waypoints);
        free(waypoint->name);
        waypoint->name = (char*)xmlNodeGetContent(cur_node);
    }
    else if (strcmp(parentName, "trk") == 0)
    {
        Track *track = getFromBack(gpxDoc->tracks);
        free(track->name);
        track->name = (char*)xmlNodeGetContent(cur_node);
    }
    else if (strcmp(parentName, "trkpt") == 0)
    {
        Track *track = getFromBack(gpxDoc->tracks);
        TrackSegment *trackSegment = getFromBack(track->segments);
        Waypoint *waypoint = getFromBack(trackSegment->waypoints);
        free(waypoint->name);
        waypoint->name = (char*)xmlNodeGetContent(cur_node);
    }
}

void setOtherData(xmlNode *cur_node, GPXdoc *gpxDoc){
    if(cur_node->content != NULL)
    {
        Route *route = getFromBack(gpxDoc->routes);
        Track *track = getFromBack(gpxDoc->tracks);
        Waypoint *waypoint = getFromBack(gpxDoc->waypoints);
        GPXData *gpxData = createGPXData((char*)(cur_node->parent)->name, (char*)cur_node->content);
        if (strcmp((char*)((cur_node->parent)->parent)->name, "wpt") == 0)
        {
            insertBack(waypoint->otherData, gpxData);
        }
        else if (strcmp((char*)((cur_node->parent)->parent)->name, "rte") == 0)
        {
            insertBack(route->otherData, gpxData);
        }
        else if (strcmp((char*)((cur_node->parent)->parent)->name, "rtept") == 0)
        {
            Waypoint *routeWaypoint = getFromBack(route->waypoints);
            insertBack(routeWaypoint->otherData, gpxData);
        }
        else if (strcmp((char*)((cur_node->parent)->parent)->name, "trk") == 0 && strcmp((char*)((cur_node->parent)->name), "trkseg") != 0)
        {
            insertBack(track->otherData, gpxData);
        }
        else if (strcmp((char*)((cur_node->parent)->parent)->name, "trkpt") == 0)
        {
            TrackSegment *trackSegment = getFromBack(track->segments);
            Waypoint *trackSegWaypoint = getFromBack(trackSegment->waypoints);
            insertBack(trackSegWaypoint->otherData, gpxData);
        }
        else
        {
            free(gpxData);
        }
    }
}

static void parse_tree(xmlNode * a_node, GPXdoc *gpxDoc)
{
    xmlNode *cur_node = NULL;

    for (cur_node = a_node; cur_node != NULL; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
            if (strcmp((char*)cur_node->name, "wpt") == 0)
            {
                insertBack(gpxDoc->waypoints, createWaypoint("\0", 0, 0 ));
            }
            else if (strcmp((char*)cur_node->name, "rte") == 0)
            {
                insertBack(gpxDoc->routes, createRoute("\0"));
            }
            else if (strcmp((char*)cur_node->name, "trk") == 0)
            {
                insertBack(gpxDoc->tracks, createTrack("\0"));
            }
            else if (strcmp((char*)cur_node->name, "trkseg") == 0 )
            {
                Track *track = getFromBack(gpxDoc->tracks);
                insertBack(track->segments, createTrackSegment());
            }
            else if (strcmp((char*)cur_node->name, "gpx") == 0 )
            {
                strcpy(gpxDoc->namespace, (char*)cur_node->ns->href);
            }
        }

        if (cur_node->type == XML_TEXT_NODE) {
            if (strcmp((char*)(cur_node->parent)->name, "name") == 0 && (cur_node->parent)->parent != NULL)
            {
                setName(cur_node, gpxDoc);
            }
            else if ((cur_node->parent)->name != NULL && ((cur_node->parent)->parent)->name != NULL && strcmp((char*)(cur_node->parent)->name, "rtept") != 0)
            {
                setOtherData(cur_node, gpxDoc);
            }      
        }
        xmlAttr *attr;
        for (attr = cur_node->properties; attr != NULL; attr = attr->next)
        {
            xmlNode *value = attr->children;
            if (strcmp((char*)attr->name, "version") == 0)
            {
                gpxDoc->version = atof((char*)value->content);
            }
            else if (strcmp((char*)attr->name, "creator") == 0)
            {
                char* creator = (char*)xmlNodeGetContent(value);
                gpxDoc->creator = creator;
            }
            else if (strcmp((char*)attr->name, "lat") == 0)
            {
                if (strcmp((char*)cur_node->name, "wpt") == 0)
                {
                    Waypoint *waypoint = getFromBack(gpxDoc->waypoints);
                    waypoint->latitude = atof((char*)value->content);
                }
                else if (strcmp((char*)cur_node->name, "rtept") == 0)
                {
                    Route *route = getFromBack(gpxDoc->routes);
                    Waypoint *waypoint = getFromBack(route->waypoints);
                    if (waypoint != NULL && waypoint->latitude == 0)
                    {
                        waypoint->latitude = atof((char*)value->content);
                    }
                    else
                    {
                        insertBack(route->waypoints, createWaypoint("\0", 0.0, atof((char*)value->content)));
                    }
                }
                else if (strcmp((char*)cur_node->name, "trkpt") == 0)
                {
                    Track *track = getFromBack(gpxDoc->tracks);
                    TrackSegment *trackSeg = getFromBack(track->segments);
                    Waypoint *waypoint = getFromBack(trackSeg->waypoints);
                    if (waypoint != NULL && waypoint->latitude == 0)
                    {
                        waypoint->latitude = atof((char*)value->content);
                    }
                    else
                    {
                        insertBack(trackSeg->waypoints, createWaypoint("\0", 0.0, atof((char*)value->content)));
                    }
                }
                
            }
            else if (strcmp((char*)attr->name, "lon") == 0)
            {
                if (strcmp((char*)cur_node->name, "wpt") == 0)
                {
                    Waypoint *waypoint = getFromBack(gpxDoc->waypoints);
                    waypoint->longitude = atof((char*)value->content);
                }
                else if (strcmp((char*)cur_node->name, "rtept") == 0)
                {
                    Route *route = getFromBack(gpxDoc->routes);
                    Waypoint *waypoint = getFromBack(route->waypoints);
                    if (waypoint != NULL && waypoint->longitude == 0)
                    {
                        waypoint->longitude = atof((char*)value->content);
                    }
                    else
                    {
                        insertBack(route->waypoints, createWaypoint("\0", atof((char*)value->content), 0.0));
                    }
                }
                else if (strcmp((char*)cur_node->name, "trkpt") == 0)
                {
                    Track *track = getFromBack(gpxDoc->tracks);
                    TrackSegment *trackSeg = getFromBack(track->segments);
                    Waypoint *waypoint = getFromBack(trackSeg->waypoints);
                    if (waypoint != NULL && waypoint->longitude == 0)
                    {
                        waypoint->longitude = atof((char*)value->content);
                    }
                    else
                    {
                        insertBack(trackSeg->waypoints, createWaypoint("\0", atof((char*)value->content), 0.0));
                    }
                }
            }
        }
        parse_tree(cur_node->children, gpxDoc);
    }
}

xmlDoc* createXMLDoc(char* filename){
    /* Read to xmlDoc ------------------------------------------------------------------- */

    /* create xmlDoc pointer */
    xmlDoc *doc = NULL;

    /* check if file name is empty or NULL */
    if(filename == NULL || filename[0] == '\0'){
        return NULL;
    }

    /* parse the file */
    doc = xmlReadFile(filename, NULL, 0);
    xmlCleanupParser();
    xmlMemoryDump();
    /* check if parsing was successful */
    if (doc == NULL) {
        return NULL;
    }

    return doc;
}

GPXdoc* createGPXdoc(char* fileName){
    if(fileName == NULL){
        return NULL;
    }

    xmlDoc *doc= createXMLDoc(fileName);
    if (doc == NULL)
    {
        xmlCleanupParser();
        return NULL;
    }
    
    /* allocate GPXDoc */
    GPXdoc *gpxDoc = malloc(sizeof(GPXdoc));
    if (gpxDoc == NULL)
    {
        return NULL;
    }
    gpxDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    gpxDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    gpxDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);

    /*Get the root element node */
    xmlNode *root_element = xmlDocGetRootElement(doc);
    parse_tree(root_element, gpxDoc);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return gpxDoc;
}


//A2---------------------------------------------------------------------------------------------------
bool validateXML(xmlDocPtr doc, char* gpxSchemaFile){
    bool valid = FALSE;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);

    schema = xmlSchemaParse(ctxt);
    xmlSchemaFreeParserCtxt(ctxt);


    if (doc == NULL)
    {
        if(schema != NULL)
        {
            xmlSchemaFree(schema);
        }
        return FALSE;
    }
    else
    {
        xmlSchemaValidCtxtPtr ctxt;
        int ret;

        ctxt = xmlSchemaNewValidCtxt(schema);
        ret = xmlSchemaValidateDoc(ctxt, doc);
        if (ret == 0)
        {
            valid = TRUE;
        }
        xmlSchemaFreeValidCtxt(ctxt);
    }

    // free the resource
    if(schema != NULL)
    {
        xmlSchemaFree(schema);
    }
    return(valid);
}

GPXdoc* createValidGPXdoc(char* fileName, char* gpxSchemaFile){
    if (fileName == NULL || gpxSchemaFile == NULL)
    {
        return NULL;
    } 
    xmlDoc *doc= createXMLDoc(fileName);
    if (doc != NULL)
    {
        if( validateXML(doc, gpxSchemaFile) == TRUE )
        {
            xmlFreeDoc(doc);
            xmlCleanupParser();
            return (createGPXdoc(fileName));
        }
        else
        {
            xmlFreeDoc(doc);
            xmlCleanupParser();
            return NULL;
        }
    }
    return NULL;
}

xmlDoc* GPXtoXMLDoc(GPXdoc* gpxDoc){
    if (gpxDoc == NULL)
    {
        return NULL;
    }
    xmlDocPtr xmlDoc = NULL;       /* xmlDocument pointer */
    xmlNodePtr root_node = NULL, node = NULL, node1 = NULL, node2 = NULL;/* node pointers */

    LIBXML_TEST_VERSION;

    xmlDoc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "gpx");
    xmlDocSetRootElement(xmlDoc, root_node);
    char* versionStr = malloc(sizeof(char)*25);
    sprintf(versionStr, "%.1f", gpxDoc->version);
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST versionStr);
    xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST gpxDoc->creator);
    xmlSetNs(root_node, xmlNewNs(root_node, (unsigned char*)gpxDoc->namespace, NULL));
    char* valueStr = malloc(sizeof(char)*50);
    List *list = gpxDoc->waypoints;
	ListIterator iter = createIterator(list), secondIter, thirdIter, fourthIter;
    Waypoint* elem;
    GPXData* data;
    if (getLength(list) != 0)
    {
        while((elem = nextElement(&iter)) != NULL){
            node = xmlNewChild(root_node, NULL, BAD_CAST "wpt", BAD_CAST "");
            secondIter = createIterator(elem->otherData);
            sprintf(valueStr, "%f", elem->latitude);
            xmlNewProp(node, BAD_CAST "lat", BAD_CAST valueStr);
            sprintf(valueStr, "%f", elem->longitude);
            xmlNewProp(node, BAD_CAST "lon", BAD_CAST valueStr);
            if (strcmp(elem->name, "") != 0)
            {
                 xmlNewChild(node, NULL, BAD_CAST "name", BAD_CAST elem->name);
            }

            while((data = nextElement(&secondIter)) != NULL){
                xmlNewChild(node, NULL, BAD_CAST data->name, BAD_CAST data->value);
            }
        }
    }
    //Routes---------------------------------------------------------------------------------
    list = gpxDoc->routes;
	iter = createIterator(list);
    Route* routeElem;
    Waypoint* wayElem;
    if (getLength(list) != 0)
    {
        while((routeElem = nextElement(&iter)) != NULL){
            node = xmlNewChild(root_node, NULL, BAD_CAST "rte", BAD_CAST "");
            secondIter = createIterator(routeElem->otherData);
            if (strcmp(routeElem->name, "") != 0)
            {
                 xmlNewChild(node, NULL, BAD_CAST "name", BAD_CAST routeElem->name);
            }
            //OtherData
            while((data = nextElement(&secondIter)) != NULL){
                xmlNewChild(node, NULL, BAD_CAST data->name, BAD_CAST data->value);
            }
            //Waypoints
            secondIter = createIterator(routeElem->waypoints);
            while((wayElem = nextElement(&secondIter)) != NULL){
                node1 = xmlNewChild(node, NULL, BAD_CAST "rtept", BAD_CAST "");
                thirdIter = createIterator(wayElem->otherData);
                sprintf(valueStr, "%f", wayElem->latitude);
                xmlNewProp(node1, BAD_CAST "lat", BAD_CAST valueStr);
                sprintf(valueStr, "%f", wayElem->longitude);
                xmlNewProp(node1, BAD_CAST "lon", BAD_CAST valueStr);
                if (strcmp(wayElem->name, "") != 0)
                {
                    xmlNewChild(node1, NULL, BAD_CAST "name", BAD_CAST wayElem->name);
                }
                while((data = nextElement(&thirdIter)) != NULL){
                    xmlNewChild(node1, NULL, BAD_CAST data->name, BAD_CAST data->value);
                }
            }
        }
    }   

    //Track---------------------------------------------------------------------------------
    list = gpxDoc->tracks;
	iter = createIterator(list);
    Track* trackElem;
    TrackSegment* segElem;
    if (getLength(list) != 0)
    {
        while((trackElem = nextElement(&iter)) != NULL){
            node = xmlNewChild(root_node, NULL, BAD_CAST "trk", BAD_CAST "");
            secondIter = createIterator(trackElem->otherData);
            if (strcmp(trackElem->name, "") != 0)
            {
                xmlNewChild(node, NULL, BAD_CAST "name", BAD_CAST trackElem->name);
            }
            //OtherData
            while((data = nextElement(&secondIter)) != NULL){
                xmlNewChild(node, NULL, BAD_CAST data->name, BAD_CAST data->value);
            }
            //Segments
            secondIter = createIterator(trackElem->segments);
            while((segElem = nextElement(&secondIter)) != NULL){
                node1 = xmlNewChild(node, NULL, BAD_CAST "trkseg", BAD_CAST "");
                //Waypoints
                thirdIter = createIterator(segElem->waypoints);
                while((wayElem = nextElement(&thirdIter)) != NULL){
                    node2 = xmlNewChild(node1, NULL, BAD_CAST "trkpt", BAD_CAST "");
                    fourthIter = createIterator(wayElem->otherData);
                    sprintf(valueStr, "%f", wayElem->latitude);
                    xmlNewProp(node2, BAD_CAST "lat", BAD_CAST valueStr);
                    sprintf(valueStr, "%f", wayElem->longitude);
                    xmlNewProp(node2, BAD_CAST "lon", BAD_CAST valueStr);
                    if (strcmp(wayElem->name, "") != 0)
                    {
                        xmlNewChild(node2, NULL, BAD_CAST "name", BAD_CAST wayElem->name);
                    }
                    while((data = nextElement(&fourthIter)) != NULL){
                        xmlNewChild(node2, NULL, BAD_CAST data->name, BAD_CAST data->value);
                    }
                }                
            }
        }
    }                   
    free(versionStr);
    free(valueStr);
    xmlCleanupParser();
    xmlMemoryDump();
    return(xmlDoc);    
}

bool writeGPXdoc(GPXdoc* doc, char* fileName){
    if(doc == NULL || fileName == NULL || strcmp(fileName, "") == 0)
    {
        return FALSE;
    }
    else
    {
        xmlDoc* xmlTree = GPXtoXMLDoc(doc);
        if (xmlTree == NULL)
        {
            return FALSE;
        }
        else
        {
            if (xmlSaveFormatFileEnc(fileName, xmlTree, NULL, 1) == -1)
            {
                return FALSE;
            }
            else
            {
                xmlCleanupParser();
                xmlMemoryDump();
                xmlFreeDoc(xmlTree);
                return TRUE;
            }
        }
    }
    return FALSE;

}

bool validateGPXDoc(GPXdoc* gpxDoc, char* gpxSchemaFile){
    bool valid = TRUE;
    if (gpxDoc == NULL || gpxSchemaFile == NULL)
    {
        return FALSE;
    }
    xmlDoc* xmlTree = GPXtoXMLDoc(gpxDoc);
    if( validateXML(xmlTree, gpxSchemaFile) == FALSE )
    {
        xmlFreeDoc(xmlTree);
        xmlCleanupParser();
        return FALSE;
    }    
    xmlFreeDoc(xmlTree);
    xmlCleanupParser();
    if ( gpxDoc->version < (float)0 || strcmp(gpxDoc->namespace, "") == 0 || gpxDoc->creator == NULL || strcmp(gpxDoc->creator, "") == 0 )
    {
        return FALSE;
    }
    if ( gpxDoc->waypoints == NULL || gpxDoc->routes == NULL || gpxDoc->tracks == NULL )
    {
        return FALSE;
    }
    else //waypoints -----------------------------------------------------------------------------------------
    {
        ListIterator iter = createIterator(gpxDoc->waypoints), secondIter ;
        Waypoint *way;
        GPXData *data;
        while((way = nextElement(&iter)) != NULL){
            if ( way->latitude <= -90.0 || way->latitude >= 90.0 || way->longitude <= -180.0 || way->longitude >= 180.0 || way->name == NULL || way->otherData == NULL)
            {
                return FALSE;
            }
            else
            {
                secondIter = createIterator(way->otherData);
                while((data = nextElement(&secondIter)) != NULL){
                    if (strcmp(data->name, "") == 0 || strcmp(data->value, "") == 0)
                    {
                        return FALSE;
                    }
                }
            }
        }        
        
    //routes -----------------------------------------------------------------------------------------
        ListIterator dataIter, wayIter;
        iter = createIterator(gpxDoc->routes);
        Route *route;
        while((route = nextElement(&iter)) != NULL){
            if ( route->name == NULL || route->waypoints == NULL || route->otherData == NULL)
            {
                return FALSE;
            }
            else
            {
                dataIter = createIterator(route->otherData);
                while((data = nextElement(&dataIter)) != NULL){
                    if (strcmp(data->name, "") == 0 || strcmp(data->value, "") == 0)
                    {
                        return FALSE;
                    }
                }

                wayIter = createIterator(route->waypoints);
                while((way = nextElement(&wayIter)) != NULL){
                    if (way->latitude <= -90.0 || way->latitude >= 90.0 || way->longitude <= -180.0 || way->longitude >= 180.0 || way->name == NULL || way->otherData == NULL )
                    {
                        return FALSE;
                    }
                    dataIter = createIterator(way->otherData);
                    while((data = nextElement(&dataIter)) != NULL){
                        if (strcmp(data->name, "") == 0 || strcmp(data->value, "") == 0)
                        {
                            return FALSE;
                        }
                    }
                }
            }
        }        
     //Tracks -----------------------------------------------------------------------------------------
        ListIterator segIter;
        iter = createIterator(gpxDoc->tracks);
        Track *track;
        TrackSegment *trackSeg;
        while((track = nextElement(&iter)) != NULL){
            if ( track->name == NULL || track->segments == NULL || track->otherData == NULL)
            {
                return FALSE;
            }
            else
            {
                dataIter = createIterator(track->otherData);
                while((data = nextElement(&dataIter)) != NULL){
                    if (strcmp(data->name, "") == 0 || strcmp(data->value, "") == 0)
                    {
                        return FALSE;
                    }
                }

                segIter = createIterator(track->segments);
                while((trackSeg = nextElement(&segIter)) != NULL){
                    if (trackSeg->waypoints == NULL)
                    {
                        return FALSE;
                    }
                    wayIter = createIterator(trackSeg->waypoints);
                    while((way = nextElement(&wayIter)) != NULL){
                        if ( way->latitude <= -90.0 || way->latitude >= 90.0 || way->longitude <= -180.0 || way->longitude >= 180.0 || way->name == NULL || way->otherData == NULL)
                        {
                            return FALSE;
                        }
                        else
                        {
                            dataIter = createIterator(way->otherData);
                            while((data = nextElement(&dataIter)) != NULL){
                                if (strcmp(data->name, "") == 0 || strcmp(data->value, "") == 0)
                                {
                                    return FALSE;
                                }
                            }
                        }
                    }
                }
            }
        }            
    }    

    return valid;
}

float round10(float len){
    return (round(len * pow(10,-1))) * 10;
}

float distanceBetween(float lon1, float lat1, float lon2, float lat2){

    double R = 6371 * pow(10,3); // metres
    double phi1 = lat1 * M_PI/180; // φ, λ in radians
    double phi2 = lat2 * M_PI/180;
    double delta_phi = (lat2-lat1) * M_PI/180;
    double delta_lambda = (lon2-lon1) * M_PI/180;

    double a = sin(delta_phi/2) * sin(delta_phi/2) +
            cos(phi1) * cos(phi2) *
            sin(delta_lambda/2) * sin(delta_lambda/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));

    double d = R * c; // in metres
    return (float)d;
}

float getRouteLen(const Route *rt){
    if(rt == NULL || getLength(rt->waypoints) == 1){
        return (float)0;
    }
    float length = 0.0;
    Waypoint *way1 = NULL, *way2 = NULL;
    ListIterator iter = createIterator(rt->waypoints);
    way1 = nextElement(&iter);
    if(way1 != NULL){
        while((way2 = nextElement(&iter)) != NULL){
            length += distanceBetween(way1->longitude, way1->latitude, way2->longitude, way2->latitude);
            way1 = way2;
        }
    }
    return length;
}

float getTrackLen(const Track *tr){
    if(tr == NULL){
        return (float)0;
    }
    float length = 0.0;
    Waypoint *way1 = NULL, *way2 = NULL;
    TrackSegment* seg1 = NULL, *seg2 = NULL;
    ListIterator segIter = createIterator(tr->segments), wayIter;

    while((seg1 = nextElement(&segIter)) != NULL){
        wayIter = createIterator(seg1->waypoints);
        way1 = nextElement(&wayIter);
        if(way1 != NULL){
            while((way2 = nextElement(&wayIter)) != NULL){
                length += distanceBetween(way1->longitude, way1->latitude, way2->longitude, way2->latitude);
                way1 = way2;
            }
        }
    }

    segIter = createIterator(tr->segments);
    seg1 = nextElement(&segIter);
    if(seg1 != NULL){
        while((seg2 = nextElement(&segIter)) != NULL){
            way1 = getFromBack(seg1->waypoints);
            way2 = getFromFront(seg2->waypoints);
            length += distanceBetween(way1->longitude, way1->latitude, way2->longitude, way2->latitude);
            seg1 = seg2;
        }        
    }
    return length;    
}

int numRoutesWithLength(const GPXdoc* doc, float len, float delta){
    if (doc == NULL || len < 0 || delta < 0 )
    {
        return 0;
    }
    int matches = 0;
    ListIterator iter = createIterator(doc->routes);
    Route *route = NULL;
    while((route = nextElement(&iter)) != NULL){
        if( fabs((getRouteLen(route) - len)) <= delta ){
            matches++;
        }
    }    
    return matches;
}

int numTracksWithLength(const GPXdoc* doc, float len, float delta){
     if (doc == NULL || len < 0 || delta < 0 )
    {
        return 0;
    }
    int matches = 0;
    ListIterator iter = createIterator(doc->tracks);
    Track *track = NULL;
    while((track = nextElement(&iter)) != NULL){
        if( fabs((getTrackLen(track) - len)) <= delta ){
            matches++;
        }
    }    
    return matches;   
}

bool isLoopRoute(const Route* route, float delta){
    if ( route == NULL || delta < 0 )
    {
        return FALSE;
    }
    else
    {
        Waypoint *way1 = NULL, *way2 = NULL;
        if (getLength(route->waypoints) >= 4)
        {
            way1 = getFromFront(route->waypoints);
            way2 = getFromBack(route->waypoints);
            if ( distanceBetween(way1->longitude, way1->latitude, way2->longitude, way2->latitude) < delta )
            {
                return TRUE;
            }
        }
        return FALSE;
    }
}

bool isLoopTrack(const Track *tr, float delta){
    if ( tr == NULL || delta < 0 )
    {
        return FALSE;
    }
    else
    {
        int numWays = 0;
        TrackSegment *seg1 = NULL, *seg2 = NULL;
        ListIterator iter = createIterator(tr->segments);
        while((seg1 = nextElement(&iter)) != NULL){
            numWays += getLength(seg1->waypoints);
        }    
        if (numWays >= 4)
        {
            Waypoint *way1 = NULL, *way2 = NULL;
            seg1 = getFromFront(tr->segments);
            seg2 = getFromBack(tr->segments);
            way1 = getFromFront(seg1->waypoints);
            way2 = getFromBack(seg2->waypoints);
            if(distanceBetween(way1->longitude, way1->latitude, way2->longitude, way2->latitude) < delta){
                return TRUE;
            }
        }
        return FALSE;
    }
}

void deleteStub(){
}

List* getRoutesBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    if (doc == NULL || delta < 0)
    {
        return NULL;
    }
    ListIterator iter = createIterator(doc->routes);
    Route *route = NULL;
    Waypoint *start = NULL, *end = NULL;
    List *list = initializeList(&routeToString, &deleteStub, &compareRoutes);
    while((route = nextElement(&iter)) != NULL){
        start = getFromFront(route->waypoints);
        end = getFromBack(route->waypoints);
        if (start != NULL || end != NULL)
        {
            if( distanceBetween(start->longitude, start->latitude, sourceLong, sourceLat) < delta &&  distanceBetween(end->longitude, end->latitude, destLong, destLat) < delta){
                insertBack(list, route);
            }
        }
    }  
    if (getLength(list) == 0)
    {
        return NULL;
    }
    else
    {
        return list;
    }   
}

List* getTracksBetween(const GPXdoc* doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    if (doc == NULL || delta < 0)
    {
        return NULL;
    }
    ListIterator iter = createIterator(doc->tracks);
    Track *track = NULL;
    TrackSegment *startSeg = NULL, *endSeg = NULL;
    Waypoint *startWay = NULL, *endWay = NULL;
    List *list = initializeList(&trackToString, &deleteStub, &compareTracks);
    while((track = nextElement(&iter)) != NULL){
        startSeg = getFromFront(track->segments);
        endSeg = getFromBack(track->segments);
        startWay = getFromFront(startSeg->waypoints);
        endWay = getFromBack(endSeg->waypoints);
        if( distanceBetween(startWay->longitude, startWay->latitude, sourceLong, sourceLat) < delta &&  distanceBetween(endWay->longitude, endWay->latitude, destLong, destLat) < delta){
            insertBack(list, track);
        }
    }  
    if (getLength(list) == 0)
    {
        return NULL;
    }
    else
    {
        return list;
    }   
}

int numPointsTrack(const Track *track){
	ListIterator iter = createIterator(track->segments);
    TrackSegment* trackSeg;
    int numPoints = 0;
    /*Tracks*/
	while((trackSeg = nextElement(&iter)) != NULL){
        numPoints += getLength(trackSeg->waypoints);
    }
    return numPoints;
}



char* trackToJSON(const Track *tr){
    char *str;
    if (tr == NULL)
    {
        str = malloc(sizeof(char) * 3);
        strcpy(str, "{}");
        return str;
    }
    else
    {   
        char *loop = malloc( sizeof(char) * strlen("false") + 1 );
        if (isLoopTrack(tr, (float)10) == TRUE)
        {
            strcpy(loop, "true");
        }
        else
        {
            strcpy(loop, "false");
        }
        char *name;
        if (strcmp(tr->name, "") == 0)
        {
            name = malloc( sizeof(char) * strlen("None") + 1 );
            strcpy(name, "None");
        }
        else
        {
            name = tr->name;
        }
        int size = strlen("{\"name\":\"\",\"numPoints\":%d,\"len\":,\"loop\":}") + strlen(name) + 22;
        str = malloc(size);
        sprintf(str, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, numPointsTrack(tr), round10(getTrackLen(tr)), loop);
        free(loop);
        if (strcmp(name, "None") == 0)
        {
            free(name);
        }
    }
    return str;
}


char* routeToJSON(const Route *rt){
    char *str;
    if (rt == NULL)
    {
        str = malloc(sizeof(char) * 3);
        strcpy(str, "{}");
        return str;
    }
    else
    {   
        char *loop = malloc( sizeof(char) * strlen("false") + 1 );
        if (isLoopRoute(rt, (float)10) == TRUE)
        {
            strcpy(loop, "true");
        }
        else
        {
            strcpy(loop, "false");
        }
        char *name;
        if (strcmp(rt->name, "") == 0)
        {
            name = malloc( sizeof(char) * strlen("None") + 1 );
            strcpy(name, "None");
        }
        else
        {
            name = rt->name;
        }
        
        int size = strlen("{\"name\":\"\",\"numPoints\":,\"len\":,\"loop\":loopStat}") + strlen(name) + 36;
        str = malloc(size);
        sprintf(str, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s}", name, getLength(rt->waypoints), round10(getRouteLen(rt)), loop);
        free(loop);
        if (strcmp(name, "None") == 0)
        {
            free(name);
        }
    }
    return str;
}

char* waypointToJSON(const Waypoint *wt){
    char *str;
    if (wt == NULL)
    {
        str = malloc(sizeof(char) * 3);
        strcpy(str, "{}");
        return str;
    }
    else
    {   
        char *name;
        if (strcmp(wt->name, "") == 0)
        {
            name = malloc( sizeof(char) * strlen("None") + 1 );
            strcpy(name, "None");
        }
        else
        {
            name = wt->name;
        }
        
        int size = strlen("{\"name\":\"\",\"lat\":,\"lon\":}") + strlen(name) + 36;
        str = malloc(size);
        sprintf(str, "{\"name\":\"%s\",\"lat\":%.7f,\"lon\":%.7f}", name, wt->latitude, wt->longitude);
        if (strcmp(name, "None") == 0)
        {
            free(name);
        }
    }
    return str;
}

char* routepointListToJSON(const List *list){
    char *str = malloc(3);
    char *temp;
    int iteration = 0;
    if (list == NULL || getLength((List*)list) == 0)
    {
        strcpy(str, "[]");
        return str;
    }
    ListIterator iter = createIterator((List*)list);
    Waypoint *waypoint = NULL;
    strcpy(str, "[");
    while((waypoint = nextElement(&iter)) != NULL){
        temp = waypointToJSON(waypoint);
        str = realloc(str, strlen(str) + strlen(temp) + 2);
        strcat(str, temp);
        iteration++;
        if ( getLength((List*)list) > 1 && iteration < getLength((List*)list) )
        {
            strcat(str, ",");
        }
        free(temp);
    }  
    strcat(str, "]");
    return str;
}

char* routeListToJSON(const List *list){
    char *str = malloc(3);
    char *temp;
    int iteration = 0;
    if (list == NULL || getLength((List*)list) == 0)
    {
        strcpy(str, "[]");
        return str;
    }
    ListIterator iter = createIterator((List*)list);
    Route *route = NULL;
    strcpy(str, "[");
    while((route = nextElement(&iter)) != NULL){
        temp = routeToJSON(route);
        str = realloc(str, strlen(str) + strlen(temp) + 2);
        strcat(str, temp);
        iteration++;
        if ( getLength((List*)list) > 1 && iteration < getLength((List*)list) )
        {
            strcat(str, ",");
        }
        free(temp);
    }  
    strcat(str, "]");
    return str;
}

char* trackListToJSON(const List *list){
    char *str = malloc(3);
    char *temp;
    int iteration = 0;
    if ( list == NULL || getLength((List*)list) == 0 )
    {
        strcpy(str, "[]");
        return str;
    }
    ListIterator iter = createIterator((List*)list);
    Track *track = NULL;
    strcpy(str, "[");
    while((track = nextElement(&iter)) != NULL){
        temp = trackToJSON(track);
        str = realloc(str, strlen(str) + strlen(temp) + 2);
        strcat(str, temp);
        iteration++;
        if ( getLength((List*)list) > 1 && iteration < getLength((List*)list) )
        {
            strcat(str, ",");
        }
        free(temp);
    }  
    strcat(str, "]");
    return str;
}



char *replaceWithSpaces(char *string){
    char *temp = malloc(sizeof(char) * strlen(string) + 1);
    strcpy(temp, string);
    char c1, c2, c3;
    for (int i = 0; i < strlen(string)-2; i++)
    {
        c1 = temp[i];
        c2 = temp[i+1];
        c3 = temp[i+2];
        if ( (c1 == '\"' && c2 == ':' && c3 == '\"') )
        {
            temp[i] = ' ';
            temp[i+1] = ' ';
            temp[i+2] = ' ';
        }
        else if ( (c2 == '\"' && c3 == '}') )
        {
            temp[i+1] = ' ';
            temp[i+2] = ' ';
        }
        else if ( (c1 == '{' && c2 == '\"') || (c1 == '\"' && c2 == ':') || (c1 == ',' && c2 == '\"') )
        {
            temp[i] = ' ';
            temp[i+1] = ' ';
        }
    }
    return temp;
}

char* GPXtoJSON(const GPXdoc* gpx){
    char *str;
    if (gpx == NULL)
    {
        str = malloc(sizeof(char) * 3);
        strcpy(str, "{}");
        return str;
    }
    else
    {   
        int size = strlen("{\"version\":ver,\"creator\":\"crVal\",\"numWaypoints\":numW,\"numRoutes\":numR,\"numTracks\":numT}") + strlen(gpx->creator) + 61;
        str = malloc(size);
        sprintf(str, "{\"version\":%.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}", gpx->version, gpx->creator, getLength(gpx->waypoints), getLength(gpx->routes),getLength(gpx->tracks));
    }
    return str;
}

void addWaypoint(Route *rt, Waypoint *pt){
    if ( rt == NULL || pt == NULL)
    {
        return;
    }
    else
    {
        insertBack(rt->waypoints, pt);
    }
}

void addRoute(GPXdoc* doc, Route* rt){
    if ( doc == NULL || rt == NULL)
    {
        return;
    }
    else
    {
        insertBack(doc->routes, rt);
    }
}

GPXdoc* JSONtoGPX(const char* gpxString){
    if (gpxString == NULL)
    {
        return NULL;
    }
    else
    {
        char *versionString;
        char *creator;
        char c1, c2, c3;
        int start = 0, check = 0;
        float version = 0;

        for (int i = 0; i < strlen(gpxString); i++)
        {
            start = 0;
            check = 0;
            c1 = gpxString[i];
            if (c1 == ':')
            {
                for (int j = i+1; j < strlen(gpxString); j++)
                {
                    if (check == 1)
                    {
                        break;
                    }
                    c2 = gpxString[j];
                    if (c2 != '\"')
                    {
                        for (int q = j; q < strlen(gpxString); q++)
                        {
                            c3 = gpxString[q];
                            if(c3 == ',')
                            {
                                versionString = malloc(sizeof(char)*(q-j)+1);
                                start = 0;
                                for (int z = j; z < q; z++)
                                {
                                    versionString[start] = gpxString[z];
                                    start++;
                                }
                                versionString[start] = '\0';
                                check = 1;
                                version = atof(versionString);
                                free(versionString);
                            }
                            if (check == 1)
                            {
                               break;
                            }
                        }
                    }
                    else
                    {
                        for (int q = j+1; q < strlen(gpxString); q++)
                        {
                            c3 = gpxString[q];
                            if(c3 == '\"')
                            {
                                creator = malloc(sizeof(char)*(q-j));
                                start = 0;
                                for (int z = j+1; z < q; z++)
                                {
                                    creator[start] = gpxString[z];
                                    start++;
                                }
                                creator[start] = '\0';
                                check = 1;
                            }
                            if (check == 1)
                            {
                               break;
                            }
                        }
                    }
                    
                    
                }
                
            }
            
        }
        
        /* allocate GPXDoc */
        GPXdoc *gpxDoc = malloc(sizeof(GPXdoc));
        if (gpxDoc == NULL)
        {
            return NULL;
        }   
        gpxDoc->version = version;
        gpxDoc->creator = malloc(sizeof(char) * strlen(creator) + 1);
        strcpy(gpxDoc->creator, creator);
        strcpy(gpxDoc->namespace, "http://www.topografix.com/GPX/1/1"); 
        gpxDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
        gpxDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
        gpxDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
        free(creator);
        return gpxDoc;
    }
}

Waypoint* JSONtoWaypoint(const char* gpxString){
    if (gpxString == NULL)
    {
        return NULL;
    }
    else
    {
        double lat = 0;
        double lon = 0;

        char *numString = malloc(1);
        char c1, c2;
        int start = 0, check = 0, latLonCheck = 0;

        for (int i = 0; i < strlen(gpxString); i++)
        {
            start = 0;
            check = 0;
            c1 = gpxString[i];
            if (c1 == ':')
            {
                for (int j = i+1; j < strlen(gpxString); j++)
                {
                    if (check == 1)
                    {
                        break;
                    }

                    for (int q = j; q < strlen(gpxString); q++)
                    {
                        c2 = gpxString[q];
                        if(c2 == ',' || c2 == '}')
                        {
                            numString = realloc(numString, sizeof(char)*(q-j)+1);
                            start = 0;
                            for (int z = j; z < q; z++)
                            {
                                numString[start] = gpxString[z];
                                start++;
                            }
                            numString[start] = '\0';
                            check = 1;
                            latLonCheck++;
                        }
                        if (latLonCheck == 1)
                        {
                            lat = strtod(numString, NULL);
                        }
                        else if (latLonCheck == 2)
                        {
                            lon = strtod(numString, NULL);
                        }                        
                        if (check == 1)
                        {
                            break;
                        }
                    }
                }
            }
        }
        free(numString);
        return createWaypoint("", lon, lat);
    }
    return NULL;
}

Route* JSONtoRoute(const char* gpxString){
    if (gpxString == NULL)
    {
        return NULL;
    }
    else
    {
        char *name;
        char c1, c2;
        int start = 0, check = 0;

        for (int i = 0; i < strlen(gpxString); i++)
        {
            start = 0;
            check = 0;
            c1 = gpxString[i];
            if (c1 == ':')
            {
                for (int j = i+1; j < strlen(gpxString); j++)
                {
                    if (check == 1)
                    {
                        break;
                    }
                    for (int q = j+1; q < strlen(gpxString); q++)
                    {
                        c2 = gpxString[q];
                        if(c2 == '\"')
                        {
                            name = malloc(sizeof(char)*(q-j));
                            start = 0;
                            for (int z = j+1; z < q; z++)
                            {
                                name[start] = gpxString[z];
                                start++;
                            }
                            name[start] = '\0';
                            check = 1;
                        }
                        if (check == 1)
                        {
                            break;
                        }
                    }
                }
            }
        }
        char tempName[strlen(name)];
        strcpy(tempName, name);
        free(name);
        return createRoute(tempName);
    }
    return NULL;
}
//A2 END------------------------------------------------------------------------------------------------

/* to string -------------------------------------------------------------------------------*/
char* GPXdocToString(GPXdoc* doc){
    if(doc == NULL){
        char *empty = malloc(1);
        strcpy(empty, "\0");
        return empty;
    }
    int size = sizeof("[GPXDOC\n     Namespace -> \n     Version -> \n     Creator -> ]\n");
    size += sizeof(char) * (strlen(doc->namespace) + strlen(doc->creator) + 16);
    char* string = malloc(size);
    if (string == NULL){
        return NULL;
    }
    sprintf(string, "[GPXDOC\n     Namespace -> %s\n     Version -> %11.6f\n     Creator -> %s]\n", doc->namespace, doc->version, doc->creator);
    char *waypointString = toString(doc->waypoints);
    char *routeString = toString(doc->routes);
    char *track = toString(doc->tracks);
    string = realloc(string, (sizeof(char) * (strlen(string) + strlen(waypointString) + strlen(routeString) + strlen(track) + 10)));
    if (string == NULL){
        return NULL;
    }
    strcat(string, waypointString);
    strcat(string, routeString);
    strcat(string, track);
    free(routeString);
    free(waypointString);
    free(track);
    return string;
}

char* trackToString(void* data){
    if(data == NULL){
        char *empty = malloc(1);
        strcpy(empty, "\0");
        return empty;
    }
    Track *track = (Track*)data;
    int size = (sizeof(char) * strlen(("\n     [Track\n          Name -> \n          TrackSegments -> \n          Track OtherData -> ]\n"))+1);
    char *segment = toString(track->segments);
    char *otherData = toString(track->otherData);
    size += (sizeof(char) * (strlen(track->name)) + strlen(segment) + strlen(otherData) + 1);
    char *string = malloc(size);
    sprintf(string, "\n     [Track\n          Name -> %s\n          TrackSegments -> %s\n          Track OtherData -> %s]\n", track->name, segment, otherData);
    free(segment);
    free(otherData);
    return string;
}

char* trackSegmentToString(void* data){
    if(data == NULL){
        char *empty = malloc(1);
        strcpy(empty, "\0");
        return empty;
    }
    TrackSegment *trackSegment = (TrackSegment *)data;
    return toString(trackSegment->waypoints);
}

char* routeToString(void* data){
    if(data == NULL){
        char *empty = malloc(1);
        strcpy(empty, "\0");
        return empty;
    }
    Route *route = (Route*)data;
    int size = strlen("\n     [Route\n          Name -> \n          Waypoints -> \n          Route OtherData -> ]\n");
    char *gpxDataString = toString(route->otherData);
    char *waypointsString = toString(route->waypoints);
    size += sizeof(char) * (strlen(route->name) + strlen(gpxDataString) + strlen(waypointsString) + 1);
    char* string = malloc(size);
    sprintf(string, "\n     [Route\n          Name -> %s\n          Waypoints -> %s\n          Route OtherData -> %s]\n", route->name, waypointsString , gpxDataString);
    free(gpxDataString);
    free(waypointsString);
    return string;    
}

char* waypointToString( void* data){
    if(data == NULL){
        char *empty = malloc(1);
        strcpy(empty, "\0");
        return empty;
    }
    Waypoint *waypoint = (Waypoint*)data;
    int size = strlen("\n          [Waypoint\n               Name -> \n               Longitude -> \n               Latitude -> ]\n");
    char *gpxDataString = toString(waypoint->otherData);
    size += sizeof(char) * (strlen(waypoint->name) + strlen(gpxDataString) + 31);
    char* string = malloc(size);
    if (string == NULL){
        return NULL;
    }
    sprintf(string, "\n          [Waypoint\n               Name -> %s\n               Longitude -> %11.6f\n               Latitude -> %11.6f]\n", waypoint->name, waypoint->longitude, waypoint->latitude);
    strcat(string, gpxDataString);
    free(gpxDataString);
    return string;
}

char* gpxDataToString( void* data){
    if(data == NULL){
        char *empty = malloc(1);
        strcpy(empty, "\0");
        return empty;
    }
    GPXData *gpxData = (GPXData*)data;
    int size = strlen("\n          GPXDATA\n          Name -> \n          Value -> \n") + 1;
    size += sizeof(char) * (strlen(gpxData->name) + strlen(gpxData->value) + 1);
    char* string = malloc(size);
    if (string == NULL){
        return NULL;
    }
    sprintf(string, "\n          GPXDATA\n          Name -> %s\n          Value -> %s\n", gpxData->name, gpxData->value);
    return string;
}

/* delete -------------------------------------------------------------------------------*/
void deleteGPXdoc(GPXdoc* doc){
    if(doc == NULL){
        return;
    }
    freeList(doc->tracks);
    freeList(doc->routes);
    freeList(doc->waypoints);
    free(doc->creator);
    free(doc);
}

void deleteTrackSegment(void* data){
    if(data == NULL){
        return;
    }
    TrackSegment *trackSegment = (TrackSegment*)data;
    freeList(trackSegment->waypoints);
    free(trackSegment);
}

void deleteTrack(void* data){
    if(data == NULL){
        return;
    }
    Track *track = (Track*)data;
    free(track->name);
    freeList(track->segments);
    freeList(track->otherData);
    free(track);
}

void deleteRoute(void* data){
    if(data == NULL){
        return;
    }
    Route *route = (Route*)data;
    freeList(route->otherData);
    freeList(route->waypoints);
    free(route->name);
    free(route);
}

void deleteWaypoint(void* data){
    if(data == NULL){
        return;
    }
    Waypoint *waypoint = (Waypoint*)data;
    freeList(waypoint->otherData);
    free(waypoint->name);
    free(waypoint);
}

void deleteGpxData( void* data){
    if(data == NULL){
        return;
    }
    GPXData *gpxData = (GPXData*)data;
    free(gpxData);
}

/* get num functions ------------------------------------------------------------------------------*/

int getNumWaypoints(const GPXdoc* doc){
    if (doc == NULL)
    {
        return 0;
    }
    List *list = doc->waypoints;
    return list->length;
}

int getNumRoutes(const GPXdoc* doc){
    if (doc == NULL)
    {
        return 0;
    }
    List *list = doc->routes;
    return list->length;
}

int getNumTracks(const GPXdoc* doc){
    if (doc == NULL)
    {
        return 0;
    }
    List *list = doc->tracks;
    return list->length;
}

int getNumSegments(const GPXdoc* doc){
    if (doc == NULL)
    {
        return 0;
    }
    int size = 0;
    List *list = doc->tracks, *segList;
    Track *track;
	ListIterator iter = createIterator(list);
    void* elem;
	while((elem = nextElement(&iter)) != NULL){
        track = (Track*)elem;
        segList = track->segments;
        size += segList->length;
	}
    return size;
}

int getNumGPXData(const GPXdoc* doc){
    if (doc == NULL)
    {
        return 0;
    }
    int size = 0;
    List *list = doc->tracks, *gpxList;
    Track *track;
    Route *route;
    Waypoint *waypoint;
    TrackSegment *trackSeg;
	ListIterator iter = createIterator(list), secondIter, thirdIter;
    void *elem, *secondElem, *thirdElem;
    /*Tracks*/
	while((elem = nextElement(&iter)) != NULL){
        track = (Track*)elem;
        if(strcmp((char*)track->name, "\0") != 0){
            size++;
        }
        gpxList = track->otherData;
        size += gpxList->length;

        secondIter = createIterator(track->segments);
        while((secondElem = nextElement(&secondIter)) != NULL){
            trackSeg = (TrackSegment*)secondElem;
            thirdIter = createIterator(trackSeg->waypoints); 
            while((thirdElem = nextElement(&thirdIter)) != NULL){
                waypoint = (Waypoint*)thirdElem;
                if(strcmp((char*)waypoint->name, "\0") != 0){
                    size++;
                }
                gpxList = waypoint->otherData;
                size += gpxList->length;
            }       
        }
	}
    /*Routes*/
    list = doc->routes;
    iter = createIterator(list);
	while((elem = nextElement(&iter)) != NULL){
        route = (Route*)elem;
        if(strcmp((char*)route->name, "\0") != 0){
            size++;
        }
        gpxList = route->otherData;
        size += gpxList->length;

        secondIter = createIterator(route->waypoints);
        while((secondElem = nextElement(&secondIter)) != NULL){
            waypoint = (Waypoint*)secondElem;
            if(strcmp((char*)waypoint->name, "\0") != 0){
                size++;
            }
            gpxList = waypoint->otherData;
            size += gpxList->length;  
        }
	}
    /*Waypoints*/
    list = doc->waypoints;
    iter = createIterator(list);
	while((elem = nextElement(&iter)) != NULL){
        waypoint = (Waypoint*)elem;
        if(strcmp((char*)waypoint->name, "\0") != 0){
            size++;
        }
        gpxList = waypoint->otherData;
        size += gpxList->length;
	}
    return size;
}

/*boolean functions--------------------------------------------------------------------------------*/
bool compareWaypointName(const void *first, const void *second){
    if (first != NULL && second != NULL){
        Waypoint *firstWaypoint = (Waypoint*)first;
        Waypoint *secondWaypoint = (Waypoint*)second;
        if(strcmp((char*)(firstWaypoint->name), (char*)(secondWaypoint->name)) == 0){
            return true;
        }
        return false;
    }
    else{
        return false;
    }
}

bool compareTrackName(const void *first, const void *second){
    if (first != NULL && second != NULL){
        Track *firstTrack = (Track*)first;
        Track* secondTrack = (Track*)second;
        if(strcmp((char*)(firstTrack->name), (char*)(secondTrack->name)) == 0){
            return true;
        }
        return false;
    }
    else{
        return false;
    }
}

bool compareRouteName(const void *first, const void *second){
    if (first != NULL && second != NULL){
        Route *firstRoute = (Route*)first;
        Route *secondRoute = (Route*)second;
        if(strcmp((char*)(firstRoute->name), (char*)(secondRoute->name)) == 0){
            return true;
        }
        return false;
    }
    else{
        return false;
    }
}

/* get num functions ------------------------------------------------------------------------------*/

Waypoint* getWaypoint(const GPXdoc* doc, char* name){
    if (doc == NULL || name == NULL)
    {
        return NULL;
    }
    Waypoint *waypoint = malloc(sizeof(Waypoint));
    waypoint->name = (char*)name;
    Waypoint *ret = findElement(doc->waypoints, &compareWaypointName, waypoint);
    free(waypoint);
    return ret;
}

Track* getTrack(const GPXdoc* doc, char* name){
    if (doc == NULL || name == NULL)
    {
        return NULL;
    }
    Track *track = malloc(sizeof(Track));
    track->name = (char*)name;
    Track *ret = findElement(doc->tracks, &compareTrackName, track);
    free(track);
    return ret;
}

Route* getRoute(const GPXdoc* doc, char* name){
    if (doc == NULL || name == NULL)
    {
        return NULL;
    }
    Route *route = malloc(sizeof(Route));
    route->name = (char*)name;
    Route *ret = findElement(doc->routes, &compareRouteName, route);
    free(route);
    return ret;
}


/* compare ------------------------------------------------------------------------------*/


int compareWaypoints(const void *first, const void *second){
    return 1;
}

int compareGpxData(const void *first, const void *second){
    return 1;
}

int compareRoutes(const void *first, const void *second){
    return 1;
}

int compareTrackSegments(const void *first, const void *second){
    return 1;
}

int compareTracks(const void *first, const void *second){
    return 1;
}


/*A3 WRAPPER FUNCTIONS----------------------------------------------------------------------*/

char* gpxFileToJson(char* fileName){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    char* str;
    if (doc == NULL)
    {
        str = malloc(strlen("ERROR: FILE NOT VALID") + 1);
        strcpy(str, "ERROR: FILE NOT VALID");
        return str;
    }
    str = GPXtoJSON(doc);
    deleteGPXdoc(doc);
    return str;
}

char* gpxFileToRouteJson(char* fileName){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    char* str;
    if (doc == NULL)
    {
        str = malloc(strlen("ERROR: FILE NOT VALID") + 1);
        strcpy(str, "ERROR: FILE NOT VALID");
        return str;
    }
    str = routeListToJSON(doc->routes);
    deleteGPXdoc(doc);
    return str;
}

char* gpxFileToTrackJson(char* fileName){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    char* str;
    if (doc == NULL)
    {
        str = malloc(strlen("ERROR: FILE NOT VALID") + 1);
        strcpy(str, "ERROR: FILE NOT VALID");
        return str;
    }
    str = trackListToJSON(doc->tracks);
    deleteGPXdoc(doc);
    return str;
}

void* getAtIndex(int index, List* list){
    ListIterator iter = createIterator(list);
    int count = 0;
    void* elem = NULL;
    while(((elem = nextElement(&iter)) != NULL) && count < index){
        count++;
    }    
    return elem;
}

char* GPXDataToJSON(const GPXData *data){
    char* str;
    int size = strlen("{\"name\":\"\",\"value\":\"\"}") + strlen(data->name) + strlen(data->value) + 20;
    str = malloc(size);
    sprintf(str, "{\"name\":\"%s\",\"value\":\"%s\"}", data->name, data->value);
    return str;
}

char* dataListToJSON(const List *list){
    char *str = malloc(3);
    char *temp;
    int iteration = 0;
    if ( list == NULL )
    {
        strcpy(str, "[]");
        return str;
    }
    ListIterator iter = createIterator((List*)list);
    GPXData *data;
    strcpy(str, "[");
    while((data = nextElement(&iter)) != NULL){
        temp = GPXDataToJSON(data);
        str = realloc(str, strlen(str) + strlen(temp) + 2);
        strcat(str, temp);
        iteration++;
        if ( getLength((List*)list) > 1 && iteration < getLength((List*)list) )
        {
            strcat(str, ",");
        }
        free(temp);
    }  
    strcat(str, "]");
    return str;
}

char* componentDataToJSON(char* fileName, char* component, int index){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    char* str = NULL;
    if (doc == NULL || component == NULL)
    {
        str = malloc(strlen("ERROR: FILE NOT VALID") + 1);
        strcpy(str, "ERROR: FILE NOT VALID");
        return str;
    }
    if (strcmp(component, "Route") == 0)
    {
        Route* route = (Route*)getAtIndex(index-1, doc->routes);
        str = dataListToJSON(route->otherData);
    }
    else if (strcmp(component, "Track") == 0)
    {
        Track* track = (Track*)getAtIndex(index-1, doc->tracks);
        str = dataListToJSON(track->otherData);
    }
    deleteGPXdoc(doc);
    return str;
}

char* editComponentName(char* fileName, char* newName, char* component, int index){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    if (doc == NULL || newName == NULL || index < 0)
    {
        char* str;
        str = malloc(strlen("FALSE") + 1);
        strcpy(str, "FALSE");
        return str;
    }
    char* name;
    if (strcmp(component, "Route") == 0)
    {
        Route* route = (Route*)getAtIndex(index-1, doc->routes);
        free(route->name);
        name = malloc(strlen(newName)+1);
        strcpy(name, newName);
        route->name = name;
    }
    else if (strcmp(component, "Track") == 0)
    {
        Track* track = (Track*)getAtIndex(index-1, doc->tracks);
        free(track->name);
        name = malloc(strlen(newName)+1);
        strcpy(name, newName);
        track->name = name;
    }
    char* result = malloc(10);
    if (validateGPXDoc(doc, "gpx.xsd") == TRUE)
    {
        writeGPXdoc(doc, fileName);
        strcpy(result, "TRUE");
    }
    else
    {
        strcpy(result, "FALSE");
    }
    deleteGPXdoc(doc);
    return result;
}

char* createGPXFromJSON(char* fileName, char* json){
    GPXdoc* doc = JSONtoGPX(json);
    char* result = malloc(10);
    if (validateGPXDoc(doc, "gpx.xsd") == TRUE)
    {
        writeGPXdoc(doc, fileName);
        strcpy(result, "TRUE");
    }
    else
    {
        strcpy(result, "FALSE");
    }
    deleteGPXdoc(doc);
    return result;
}

char* addRouteToFile(char* fileName, char* json){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    char* str = NULL;
    if (doc == NULL || json == NULL)
    {
        str = malloc(strlen("ERROR: FILE NOT VALID") + 1);
        strcpy(str, "ERROR: FILE NOT VALID");
        return str;
    }
    addRoute(doc, JSONtoRoute(json));
    char* result = malloc(10);
    if (validateGPXDoc(doc, "gpx.xsd") == TRUE)
    {
        writeGPXdoc(doc, fileName);
        strcpy(result, "TRUE");
    }
    else
    {
        strcpy(result, "FALSE");
    }
    deleteGPXdoc(doc);
    return result;
}

char* addWaypointToRoute(char* fileName, char* routeName, char* json){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    char* str = NULL;
    if (doc == NULL || routeName == NULL || json == NULL)
    {
        str = malloc(strlen("FALSE") + 1);
        strcpy(str, "FALSE");
        return str;
    }
    char* result = malloc(10);
    if (getRoute(doc, routeName) == NULL || JSONtoWaypoint(json) == NULL)
    {
        strcpy(result, "FALSE");
        return result;
    }
    else
    {
            Route* route = getRoute(doc, routeName);
        addWaypoint(route, JSONtoWaypoint(json));
        
        if (validateGPXDoc(doc, "gpx.xsd") == TRUE)
        {
            writeGPXdoc(doc, fileName);
            strcpy(result, "TRUE");
        }
        else
        {
            strcpy(result, "FALSE");
        }
        deleteGPXdoc(doc);
        return result;
    }
}

char* findRoutePathData(char* fileName, double sourceLat, double sourceLong, double destLat, double destLong, double delta){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    char* str = NULL;
    if (doc == NULL)
    {
        str = malloc(strlen("FALSE") + 1);
        strcpy(str, "FALSE");
        return str;
    }
    List *routes;
    routes = getRoutesBetween(doc, sourceLat, sourceLong, destLat, destLong, delta);
    str = routeListToJSON(routes);
    
    if (str == NULL)
    {
        char* result = malloc(10);
        strcpy(result, "FALSE");
        return result;
    }
    deleteGPXdoc(doc);
    return str;
}

char* findTrackPathData(char* fileName, float sourceLat, float sourceLong, float destLat, float destLong, float delta){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    char* str = NULL;
    if (doc == NULL)
    {
        str = malloc(strlen("FALSE") + 1);
        strcpy(str, "FALSE");
        return str;
    }
    List *tracks;
    tracks = getTracksBetween(doc, sourceLat, sourceLong, destLat, destLong, delta);

    str = trackListToJSON(tracks);

    if (str == NULL)
    {
        char* result = malloc(10);
        strcpy(result, "FALSE");
        return result;
    }
    deleteGPXdoc(doc);
    return str;
}

char* routepointsToJSON(char* fileName, int index){
    GPXdoc* doc = createValidGPXdoc(fileName, "gpx.xsd");
    char* str = NULL;
    if (doc == NULL || index < 0)
    {
        str = malloc(strlen("ERROR: FILE NOT VALID") + 1);
        strcpy(str, "ERROR: FILE NOT VALID");
        return str;
    }
    else
    {
        Route* route = (Route*)getAtIndex(index-1, doc->routes);
        str = routepointListToJSON(route->waypoints);
    }
    deleteGPXdoc(doc);
    return str;
}
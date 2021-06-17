int insertData(char tableName[], char data[][20], int colAmount)
{
    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "a+");
    int counter = 0;
    while (counter < colAmount)
    {
        if (counter > 0)
            fprintf(fpTable, ",");
        fprintf(fpTable, "%s", data[counter++]);
    }
    fprintf(fpTable, "\n");
    fclose(fpTable);
    return 1;
}

int updateData(char tableName[], char column[], char newValue[], char condValue[])
{
    int res = 0;
    int whereCond = 0;

    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "r");

    char headTable[100];
    fscanf(fpTable, "%s", headTable);

    char columnNames[100][100];
    char tempHeadTable[100];

    strcpy(tempHeadTable, headTable);
    int colAmount = splitString(columnNames, headTable);

    if (strcmp(condValue, ""))
        whereCond = 1;

    int counter = 0;
    while (counter < colAmount)
    {
        if (!strcmp(columnNames[counter], column))
        {
            res = 1;
            break;
        }

        counter++;
    }

    if (res)
    {
        char *tempFileName = "temp.csv";
        FILE *fpTemp = fopen(tempFileName, "w");
        fputs(tempHeadTable, fpTemp);

        char line[100];
        while (fgets(line, sizeof(line), fpTable) != NULL)
        {
            char columnDatas[100][100];
            colAmount = splitString(columnDatas, line);
            int chunkCounter = 0;
            while (chunkCounter < colAmount)
            {
                if (chunkCounter > 0)
                    fprintf(fpTemp, ",");

                if (chunkCounter != counter)
                {
                    fprintf(fpTemp, "%s", columnDatas[chunkCounter]);
                }
                else
                {
                    if (!whereCond || (whereCond && !strcmp(columnDatas[chunkCounter], condValue)))
                        fprintf(fpTemp, "%s", newValue);
                    else
                        fprintf(fpTemp, "%s", columnDatas[chunkCounter]);
                }

                chunkCounter++;
            }
            fprintf(fpTemp, "\n");
        }

        fclose(fpTable);
        fclose(fpTemp);

        remove(path);
        rename(tempFileName, path);
    }
    return res;
}

int deleteData(char tableName[], char condColumn[], char condValue[])
{
    int res = 0;
    int whereCond = 0;

    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "r");

    char headTable[100];
    fscanf(fpTable, "%s", headTable);

    char columnNames[100][100];
    char tempHeadTable[100];

    strcpy(tempHeadTable, headTable);
    int colAmount = splitString(columnNames, tempHeadTable);

    if (strcmp(condColumn, ""))
        whereCond = 1;

    int counter = 0;
    if (whereCond)
    {
        while (counter < colAmount)
        {
            if (!strcmp(columnNames[counter], condColumn))
            {
                break;
            }

            counter++;
        }
    }

    char *tempFileName = "temp.csv";
    FILE *fpTemp = fopen(tempFileName, "w");
    fputs(headTable, fpTemp);

    if (whereCond)
    {
        char line[100];
        while (fgets(line, sizeof(line), fpTable) != NULL)
        {
            char columnDatas[100][100];
            char tempLine[100];
            strcpy(tempLine, line);
            colAmount = splitString(columnDatas, tempLine);

            if (strcmp(condValue, columnDatas[counter]))
                fputs(line, fpTemp);
        }
    }

    fclose(fpTable);
    fclose(fpTemp);

    remove(path);
    rename(tempFileName, path);
    res = 1;

    return res;
}

int dmlInterface(char *buffer)
{
    char query[100];
    strcpy(query, buffer);
    char splitted[100][100];
    memset(splitted, '\0', sizeof(splitted));
    int amount = splitString(splitted, query);

    int res = 0;
    if (!strcmp(splitted[0], "INSERT"))
    {
        if (!strcmp(splitted[1], "INTO"))
        {
            char tableName[20];
            strcpy(tableName, splitted[2]);
            int splitCounter = 3, colCounter = 0;
            char data[100][20];
            while (splitCounter < amount)
            {
                strcpy(data[colCounter++], splitted[splitCounter++]);
            }
            if (insertData(tableName, data, amount - 3))
                res = 1;
        }
    }
    else if (!strcmp(splitted[0], "UPDATE"))
    {
        char tableName[20];
        strcpy(tableName, splitted[1]);
        if (!strcmp(splitted[2], "SET"))
        {
            if (updateData(tableName, splitted[3], splitted[4], splitted[7]))
                res = 1;
        }
    }
    else if (!strcmp(splitted[0], "DELETE"))
    {
        if (!strcmp(splitted[1], "FROM"))
        {
            char tableName[20];
            strcpy(tableName, splitted[2]);

            if (deleteData(tableName, splitted[4], splitted[5]))
                res = 1;
        }
    }
    else if (!strcmp(splitted[0], "SELECT"))
    {
        // not implemented
    }
    return res;
}
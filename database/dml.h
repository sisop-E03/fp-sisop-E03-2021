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

int updateData(char tableName[], char column[], char newValue[],
               char condColumn[], char condValue[])
{
    int res = 0;
    int whereCond = 0;

    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "r");

    char headTable[100];
    fgets(headTable, sizeof(headTable), fpTable);

    char columnNames[100][100];
    char tempHeadTable[100];

    strcpy(tempHeadTable, headTable);
    int colAmount = splitString(columnNames, tempHeadTable);

    if (strcmp(condValue, ""))
        whereCond = 1;

    int counter = 0;
    int colIndex = -1;
    int condColIndex = -1;
    while (counter < colAmount)
    {
        if (!strcmp(columnNames[counter], column))
        {
            res = 1;
            colIndex = counter;
        }

        if (!strcmp(columnNames[counter], condColumn))
            condColIndex = counter;

        if (res && (!whereCond || (whereCond && condColIndex > -1)))
            break;

        counter++;
    }

    if (res)
    {
        char *tempFileName = "temp.csv";
        FILE *fpTemp = fopen(tempFileName, "w");
        fputs(headTable, fpTemp);

        char line[100];
        fgets(line, sizeof(line), fpTable);
        fputs(line, fpTemp);

        while (fgets(line, sizeof(line), fpTable) != NULL)
        {
            char columnDatas[100][100];
            char tempLine[100];
            strcpy(tempLine, line);
            colAmount = splitString(columnDatas, tempLine);

            if (whereCond && strcmp(condValue, columnDatas[condColIndex]))
            {
                fputs(line, fpTemp);
                continue;
            }

            int chunkCounter = 0;
            while (chunkCounter < colAmount)
            {
                if (chunkCounter > 0)
                    fprintf(fpTemp, ",");

                if (chunkCounter != colIndex)
                {
                    fprintf(fpTemp, "%s", columnDatas[chunkCounter]);
                }
                else
                {
                    fprintf(fpTemp, "%s", newValue);
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
    fgets(headTable, sizeof(headTable), fpTable);

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

    char line[100];
    fgets(line, sizeof(line), fpTable);
    fputs(line, fpTemp);

    if (whereCond)
    {
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

int selectData(char tableName[], char columns[][100], char condColumn[], char condValue[])
{
    int res = 0;
    int whereCond = 0;

    char path[100];
    sprintf(path, "databases/%s/%s.csv", activeDB, tableName);
    FILE *fpTable = fopen(path, "r");

    char headTable[100];
    fgets(headTable, sizeof(headTable), fpTable);

    char columnNames[100][100];

    int colAmount = splitString(columnNames, headTable);

    if (strcmp(condValue, ""))
        whereCond = 1;

    int all = 0;

    if (!strcmp(columns[0], "*"))
        all = 1;

    int colIndex[100];
    memset(colIndex, -1, sizeof(colIndex));
    int condColIndex = -1;
    int counter = 0;
    int i = 0;
    while (counter < colAmount)
    {
        if (all)
        {
            colIndex[i++] = counter;
            printf("%s\t", columnNames[counter]);
        }
        else
        {
            int j = 0;
            while (strcmp(columns[j], ""))
            {
                if (!strcmp(columnNames[counter], columns[j]))
                {
                    colIndex[i++] = counter;
                    printf("%s\t", columnNames[counter]);
                }
                j++;
            }
        }

        if (!strcmp(columnNames[counter], condColumn))
            condColIndex = counter;

        counter++;
    }
    printf("\n");

    char line[100];
    fgets(line, sizeof(line), fpTable);
    while (fgets(line, sizeof(line), fpTable) != NULL)
    {
        char columnDatas[100][100];
        colAmount = splitString(columnDatas, line);

        int show = 0;
        if (!whereCond || (whereCond && !strcmp(condValue, columnDatas[condColIndex])))
        {
            int chunkCounter = 0;
            while (chunkCounter < colAmount)
            {
                int i = 0;
                while (colIndex[i] != -1)
                {
                    if (chunkCounter == colIndex[i])
                    {
                        int lastChar = strlen(columnDatas[chunkCounter]) - 1;
                        if (columnDatas[chunkCounter][lastChar] == '\'')
                            columnDatas[chunkCounter][lastChar] = '\0';

                        if (columnDatas[chunkCounter][0] == '\'')
                            printf("%s\t", columnDatas[chunkCounter] + 1);
                        else
                            printf("%s\t", columnDatas[chunkCounter]);
                        break;
                    }

                    i++;
                }

                chunkCounter++;
            }
            printf("\n");
        }
    }

    fclose(fpTable);

    return 1;
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
            if (updateData(tableName, splitted[3], splitted[4], splitted[6], splitted[7]))
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
        int i = 1;
        char tableName[100];
        char columns[100][100];
        memset(columns, '\0', sizeof(columns));
        char condColumn[100];
        char condValue[100];
        while (strcmp(splitted[i], "") && strcmp(splitted[i], "FROM"))
        {
            strcpy(columns[i - 1], splitted[i]);
            i++;
        }

        strcpy(tableName, splitted[i + 1]);
        strcpy(condColumn, splitted[i + 3]);
        strcpy(condValue, splitted[i + 4]);

        if (selectData(tableName, columns, condColumn, condValue))
            res = 1;
    }
    return res;
}
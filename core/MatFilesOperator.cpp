#include "MatFilesOperator.h"

#include <QColor>
#include <QDebug>
#include <QFileInfo>

#include "libs/matio/src/matio.h"

const QString matlabAppDirRelativeName = "SpectraClassifier/application";
const QString matlabAppExeFile = "SpectraClassifier.exe";
const QString matFileName = "data_from_cpp_app.mat";

namespace {

void writeBoolAsUint8(mat_t *matfp, const char *name, bool value) {
    unsigned char v = value ? 1 : 0;
    size_t dims[2] = {1, 1};

    matvar_t *matVar = Mat_VarCreate(name,
                                     MAT_C_UINT8,  // класс: uint8
                                     MAT_T_UINT8,  // тип: uint8
                                     2, dims, &v, 0);

    Mat_VarWrite(matfp, matVar, MAT_COMPRESSION_NONE);
    Mat_VarFree(matVar);
}

// Создаёт MATLAB-переменную char (MAT_T_UTF16) с заданным именем и содержимым
// str
static matvar_t *createMatStringVar(const char *name, const QString &str) {
    // Собираем буфер 16-битных кодовых единиц UTF-16LE
    QVector<uint16_t> buf;
    buf.reserve(str.size());
    for (QChar qc : str) buf.append(qc.unicode());

    // Размеры: 1×N символов
    size_t dims[2] = {1, static_cast<size_t>(str.size())};

    // Создаём переменную MATLAB char (UTF-16)
    return Mat_VarCreate(name,         // имя переменной в .mat
                         MAT_C_CHAR,   // класс: char
                         MAT_T_UTF16,  // тип: UTF-16
                         2, dims,
                         buf.constData(),  // данные UTF-16LE
                         0);
}

QString readUtf8StringFromMat(mat_t *matfp, const QString &varName) {
    matvar_t *matvar = Mat_VarRead(matfp, varName.toUtf8().constData());
    if (!matvar) {
        qWarning() << "Variable" << varName << "not found";
        return QString();
    }
    // Проверяем что это массив uint8
    if (matvar->class_type != MAT_C_UINT8 || matvar->data_type != MAT_T_UINT8) {
        Mat_VarFree(matvar);
        qWarning() << "Variable is not uint8 array";
        return QString();
    }

    // Определяем общее количество байт
    size_t totalBytes = 1;
    for (int i = 0; i < matvar->rank; i++) {
        totalBytes *= matvar->dims[i];
    }

    QString result;
    if (totalBytes > 0) {
        // Создаем QByteArray из данных
        QByteArray byteArray(static_cast<const char *>(matvar->data),
                             totalBytes);

        // Конвертируем из UTF-8 в QString
        result = QString::fromUtf8(byteArray);
    }

    Mat_VarFree(matvar);
    return result;
}

QStringList readSpecNames(mat_t *matfp, const QString &varName) {
    QStringList specNames;
    matvar_t *var = Mat_VarRead(matfp, varName.toUtf8().constData());
    if (!var || var->class_type != MAT_C_CELL) {
        Mat_VarFree(var);
        return specNames;
    }

    // считаем общее число элементов в cell
    size_t nCells = 1;
    for (int d = 0; d < var->rank; ++d) nCells *= var->dims[d];

    matvar_t **cells = static_cast<matvar_t **>(var->data);
    specNames.reserve(int(nCells));

    for (size_t i = 0; i < nCells; ++i) {
        matvar_t *cell = cells[i];
        if (!cell || cell->class_type != MAT_C_CHAR) {
            specNames << QString();
            continue;
        }

        // Ветвление по типу кодирования
        if (cell->data_type == MAT_T_UINT8) {
            // 8-битный массив — UTF-8
            QByteArray raw8(reinterpret_cast<const char *>(cell->data),
                            int(cell->nbytes));
            specNames << QString::fromUtf8(raw8);
        } else if (cell->data_type == MAT_T_UINT16) {
            // 16-битный массив — UTF-16/UCS-2
            const quint16 *src = reinterpret_cast<const quint16 *>(cell->data);
            int nChars = int(cell->nbytes / sizeof(quint16));

            QVector<quint16> codes;
            codes.reserve(nChars);
            for (int k = 0; k < nChars; ++k) {
                quint16 code = src[k];
                // Если нужно учесть эндianness:
                // bool be = (reinterpret_cast<const uchar*>(src)[0] == 0x00);
                // if (be) code = qFromBigEndian<quint16>(code);
                if (code == 0) break;  // нуль-терминатор
                codes.append(code);
            }
            specNames << QString::fromUtf16(codes.constData(), codes.size());
        } else {
            // неизвестный формат
            specNames << QString();
        }
    }

    Mat_VarFree(var);
    return specNames;
}

QVector<int> readSelectedClustIndxs(mat_t *matfp, const QString &varName) {
    QVector<int> idxs;
    matvar_t *var = Mat_VarRead(matfp, varName.toUtf8().constData());
    if (!var) {
        qWarning() << "selectedClustIndxs not found";
        return idxs;
    }
    if (var->class_type != MAT_C_DOUBLE) {
        qWarning() << "selectedClustIndxs has wrong class_type";
        Mat_VarFree(var);
        return idxs;
    }

    // общее число элементов
    size_t nElems = 1;
    for (int i = 0; i < var->rank; ++i) nElems *= var->dims[i];

    double *data = static_cast<double *>(var->data);
    idxs.reserve(int(nElems));
    for (size_t i = 0; i < nElems; ++i) idxs.append(int(data[i]));

    Mat_VarFree(var);
    return idxs;
}

QVector<QColor> readSelectedPointsRGB(mat_t *matfp, const QString &varName) {
    QVector<QColor> colors;

    // 2. Читаем переменную selectedPointsRGB
    matvar_t *var = Mat_VarRead(matfp, varName.toUtf8().constData());
    if (!var) {
        qWarning() << "Variable 'selectedPointsRGB' not found";
        return colors;
    }

    // 3. Проверяем размерность: должен быть массив size N×3
    if (var->rank != 2 || var->dims[1] != 3) {
        qWarning() << "selectedPointsRGB has wrong dimensions:"
                   << "rank =" << var->rank << "dims[0]=" << var->dims[0]
                   << "dims[1]=" << var->dims[1];
        Mat_VarFree(var);
        return colors;
    }

    size_t N = var->dims[0];  // число точек

    // 4. Декодируем данные
    if (var->class_type == MAT_C_DOUBLE && var->data_type == MAT_T_DOUBLE) {
        // MATLAB по умолчанию хранит double
        double *data = static_cast<double *>(var->data);
        colors.reserve(int(N));
        for (size_t i = 0; i < N; ++i) {
            double r = data[i];          // первый столбец
            double g = data[N + i];      // второй столбец
            double b = data[2 * N + i];  // третий столбец
            colors.append(QColor::fromRgbF(r, g, b));
        }
    } else if (var->class_type == MAT_C_SINGLE &&
               var->data_type == MAT_T_SINGLE) {
        // если кто-то сохранял как float
        float *data = static_cast<float *>(var->data);
        colors.reserve(int(N));
        for (size_t i = 0; i < N; ++i) {
            float r = data[i];
            float g = data[N + i];
            float b = data[2 * N + i];
            colors.append(QColor::fromRgbF(r, g, b));
        }
    } else {
        qWarning() << "Unsupported data type for selectedPointsRGB:"
                   << "class_type =" << var->class_type
                   << "data_type =" << var->data_type;
    }

    // 5. Освобождаем ресурсы
    Mat_VarFree(var);
    return colors;
}

QVector<int> readIntVector(mat_t *matfp, const QString &varName) {
    QVector<int> values;

    matvar_t *var = Mat_VarRead(matfp, varName.toUtf8().constData());
    if (!var) {
        qWarning() << "Variable" << varName << "not found";
        return values;
    }

    size_t nElems = 1;
    for (int i = 0; i < var->rank; ++i) {
        nElems *= var->dims[i];
    }

    values.reserve(int(nElems));

    if (var->class_type == MAT_C_DOUBLE && var->data_type == MAT_T_DOUBLE) {
        double *data = static_cast<double *>(var->data);
        for (size_t i = 0; i < nElems; ++i) values.append(int(data[i]));
    } else if (var->class_type == MAT_C_INT32 &&
               var->data_type == MAT_T_INT32) {
        qint32 *data = static_cast<qint32 *>(var->data);
        for (size_t i = 0; i < nElems; ++i) values.append(int(data[i]));
    } else if (var->class_type == MAT_C_UINT32 &&
               var->data_type == MAT_T_UINT32) {
        quint32 *data = static_cast<quint32 *>(var->data);
        for (size_t i = 0; i < nElems; ++i) values.append(int(data[i]));
    } else {
        qWarning() << "Unsupported type for" << varName
                   << "class_type =" << var->class_type
                   << "data_type =" << var->data_type;
    }

    Mat_VarFree(var);
    return values;
}
}  // namespace

MatFilesOperator::MatFilesOperator() {}

void MatFilesOperator::saveBecasDataToMatFile(const QList<QString> &specNames,
                                              const QString &pathFolderName,
                                              bool isReflectance,
                                              const QString &fullMatPath) {
    // Открываем файл MAT5
    mat_t *matfp =
        Mat_CreateVer(fullMatPath.toUtf8().constData(), nullptr, MAT_FT_MAT5);
    if (!matfp) {
        qWarning() << "Не удалось открыть MAT-файл:" << fullMatPath;
        return;
    }

    // Создаём cell-массив N×1
    size_t dimsCell[2] = {static_cast<size_t>(specNames.size()), 1};
    matvar_t *cellArr = Mat_VarCreate("specNames", MAT_C_CELL, MAT_T_CELL, 2,
                                      dimsCell, nullptr, 0);

    // Заполняем ячейки именами файлов
    for (size_t i = 0; i < dimsCell[0]; ++i) {
        const QString &s = specNames.at(static_cast<int>(i));
        matvar_t *strVar =
            createMatStringVar(nullptr, s);  // имя внутри cell не нужно
        Mat_VarSetCell(cellArr, i, strVar);
    }
    Mat_VarWrite(matfp, cellArr, MAT_COMPRESSION_NONE);
    Mat_VarFree(cellArr);

    // Записываем строковую переменную — путь к папке
    matvar_t *folderVar = createMatStringVar("pathFolderName", pathFolderName);
    Mat_VarWrite(matfp, folderVar, MAT_COMPRESSION_NONE);
    Mat_VarFree(folderVar);

    writeBoolAsUint8(matfp, "isReflectance", isReflectance);

    Mat_Close(matfp);
    qDebug() << "Сохранено" << specNames.size()
             << "имён файлов и путь к папке в" << fullMatPath;
}

BecasDataFromMatlab MatFilesOperator::readBecasDataFromMatlab(
    const QString &fullMatPath) {
    BecasDataFromMatlab answerStruct;
    mat_t *matfp = Mat_Open(fullMatPath.toUtf8().constData(), MAT_ACC_RDONLY);
    if (!matfp) {
        qWarning() << "Failed to open MAT file:" << fullMatPath;
        answerStruct.isSomeErrors = true;
        return answerStruct;
    }
    qDebug() << "Matlab app отправил сообщение с файлом по адресу: "
             << fullMatPath;
    QString folderPath = readUtf8StringFromMat(matfp, "folderPath");
    QStringList specNamesList = readSpecNames(matfp, "specNames");
    QVector<int> classIndexes =
        readSelectedClustIndxs(matfp, "selectedClustIndxs");
    QVector<QColor> colors = readSelectedPointsRGB(matfp, "selectedPointsRGB");

    if (folderPath.isEmpty() || specNamesList.isEmpty() ||
        classIndexes.isEmpty()) {
        answerStruct.isSomeErrors = true;
    }
    Mat_Close(matfp);

    QStringList baseNames;
    for (const QString &specName : specNamesList) {
        // Разделяем имя файла и расширение
        QString baseName =
            specName.section('.', 0, -2);  // Берем все до последней точки
        baseNames.append(baseName);
    }

    answerStruct.pathFolderName = folderPath;
    answerStruct.specNamesWithExtens = specNamesList;
    answerStruct.specNames = baseNames;
    answerStruct.selectedClustIndxs = classIndexes;
    answerStruct.colorsOfEachSpectr = colors;
    return answerStruct;
}

void MatFilesOperator::saveMultiSpecDataToMatFile(
    const QVector<double> &waves, const QVector<int> &x, const QVector<int> &y,
    const QVector<QVector<double>> &specs, const QString &fullMatPath) {
    mat_t *matfp =
        Mat_CreateVer(fullMatPath.toUtf8().constData(), nullptr, MAT_FT_MAT5);
    if (!matfp) {
        qWarning() << "Не удалось открыть MAT-файл:" << fullMatPath;
        return;
    }

    const int nChan = waves.size();
    const int nX = x.size();
    const int nY = y.size();
    const int nSpec = specs.size();

    if (nChan <= 0 || nSpec <= 0) {
        qWarning() << "Empty waves/specs:"
                   << "waves=" << nChan << "specs=" << nSpec;
        Mat_Close(matfp);
        return;
    }

    // Проверим, что каждый спектр той же длины, что waves
    for (int i = 0; i < nSpec; ++i) {
        if (specs[i].size() != nChan) {
            qWarning() << "specs[" << i
                       << "] size != waves.size():" << specs[i].size() << "vs"
                       << nChan;
            Mat_Close(matfp);
            return;
        }
    }

    auto writeRowVectorDouble = [&](const char *name,
                                    const QVector<double> &v) {
        size_t dims[2] = {1, static_cast<size_t>(v.size())};
        matvar_t *var = Mat_VarCreate(name, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims,
                                      const_cast<double *>(v.constData()), 0);
        if (!var) {
            qWarning() << "Mat_VarCreate failed for" << name;
            return;
        }
        Mat_VarWrite(matfp, var, MAT_COMPRESSION_NONE);
        Mat_VarFree(var);
    };
    auto writeRowVectorInt32 = [&](const char *name, const QVector<int> &v) {
        // MATLAB int32
        QVector<qint32> tmp;
        tmp.reserve(v.size());
        for (int val : v) tmp.push_back(static_cast<qint32>(val));

        size_t dims[2] = {1, static_cast<size_t>(tmp.size())};
        matvar_t *var = Mat_VarCreate(name, MAT_C_INT32, MAT_T_INT32, 2, dims,
                                      tmp.data(), 0);
        if (!var) {
            qWarning() << "Mat_VarCreate failed for" << name;
            return;
        }
        Mat_VarWrite(matfp, var, MAT_COMPRESSION_NONE);
        Mat_VarFree(var);
    };

    writeRowVectorDouble("waves", waves);
    writeRowVectorInt32("x", x);
    writeRowVectorInt32("y", y);

    // specs: Nspec x Nchan (MATLAB column-major)
    QVector<double> buf;
    buf.resize(nSpec * nChan);

    for (int r = 0; r < nSpec; ++r) {
        for (int c = 0; c < nChan; ++c) {
            buf[r + c * nSpec] = specs[r][c];  // column-major
        }
    }

    size_t dimsSpecs[2] = {static_cast<size_t>(nSpec),
                           static_cast<size_t>(nChan)};
    matvar_t *specsVar = Mat_VarCreate("specs", MAT_C_DOUBLE, MAT_T_DOUBLE, 2,
                                       dimsSpecs, buf.data(), 0);
    if (!specsVar) {
        qWarning() << "Mat_VarCreate failed for specs";
        Mat_Close(matfp);
        return;
    }

    Mat_VarWrite(matfp, specsVar, MAT_COMPRESSION_NONE);
    Mat_VarFree(specsVar);

    Mat_Close(matfp);

    qDebug() << "Saved multispec mat:" << fullMatPath << "nSpec=" << nSpec
             << "nChan=" << nChan << "x=" << nX << "y=" << nY;
}

MultiSpecDataFromMatlab MatFilesOperator::readMultiSpecDataFromMatlab(
    const QString &fullMatPath) {
    MultiSpecDataFromMatlab answerStruct;

    mat_t *matfp = Mat_Open(fullMatPath.toUtf8().constData(), MAT_ACC_RDONLY);
    if (!matfp) {
        qWarning() << "Failed to open MAT file:" << fullMatPath;
        answerStruct.isSomeErrors = true;
        return answerStruct;
    }

    qDebug() << "Matlab app отправил multispec файл:" << fullMatPath;

    QVector<int> pixelX = readIntVector(matfp, "pixelsX");
    QVector<int> pixelY = readIntVector(matfp, "pixelsY");
    QVector<int> classIndexes =
        readSelectedClustIndxs(matfp, "selectedClustIndxs");
    QVector<QColor> colors = readSelectedPointsRGB(matfp, "selectedPointsRGB");

    Mat_Close(matfp);

    if (pixelX.isEmpty() || pixelY.isEmpty() || classIndexes.isEmpty() ||
        colors.isEmpty()) {
        answerStruct.isSomeErrors = true;
        return answerStruct;
    }

    if (pixelX.size() != pixelY.size() ||
        pixelX.size() != classIndexes.size() ||
        pixelX.size() != colors.size()) {
        qWarning() << "Multispec arrays size mismatch:"
                   << "pixelsX =" << pixelX.size()
                   << "pixelsY =" << pixelY.size()
                   << "selectedClustIndxs =" << classIndexes.size()
                   << "selectedPointsRGB =" << colors.size();
        answerStruct.isSomeErrors = true;
        return answerStruct;
    }

    answerStruct.pixelX = pixelX;
    answerStruct.pixelY = pixelY;
    answerStruct.selectedClustIndxs = classIndexes;
    answerStruct.colorsOfEachSpectr = colors;

    return answerStruct;
}

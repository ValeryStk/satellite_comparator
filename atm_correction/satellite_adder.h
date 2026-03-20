#ifndef SATELLITE_ADDER_H
#define SATELLITE_ADDER_H
#include "vector"
class QString;

namespace cat {
void loadList(QString path, std::vector<double>& list);
bool add_new_satellite(const QString& pathToFolder);
}

#endif // SATELLITE_ADDER_H

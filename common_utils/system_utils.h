#ifndef SYSTEM_UTILS_H
#define SYSTEM_UTILS_H

#include <QString>

namespace su {

/**
 * @brief Открывает переданный текст в стандартном Блокноте Windows через буфер обмена.
 * @param text Текст для отображения.
 */
void openInNotepad(const QString &text);

} // namespace su

#endif // SYSTEM_UTILS_H

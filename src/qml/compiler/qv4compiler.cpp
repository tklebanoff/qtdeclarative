/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <qv4compiler_p.h>
#include <qv4compileddata_p.h>
#include <qv4isel_p.h>
#include <qv4engine_p.h>

QV4::Compiler::JSUnitGenerator::JSUnitGenerator(QQmlJS::V4IR::Module *module)
    : irModule(module)
    , stringDataSize(0)
    , jsClassDataSize(0)
{
}

int QV4::Compiler::JSUnitGenerator::registerString(const QString &str)
{
    QHash<QString, int>::ConstIterator it = stringToId.find(str);
    if (it != stringToId.end())
        return *it;
    stringToId.insert(str, strings.size());
    strings.append(str);
    stringDataSize += QV4::CompiledData::String::calculateSize(str);
    return strings.size() - 1;
}

int QV4::Compiler::JSUnitGenerator::getStringId(const QString &string) const
{
    Q_ASSERT(stringToId.contains(string));
    return stringToId.value(string);
}

uint QV4::Compiler::JSUnitGenerator::registerGetterLookup(const QString &name)
{
    CompiledData::Lookup l;
    l.type_and_flags = CompiledData::Lookup::Type_Getter;
    l.nameIndex = registerString(name);
    lookups << l;
    return lookups.size() - 1;
}

uint QV4::Compiler::JSUnitGenerator::registerSetterLookup(const QString &name)
{
    CompiledData::Lookup l;
    l.type_and_flags = CompiledData::Lookup::Type_Setter;
    l.nameIndex = registerString(name);
    lookups << l;
    return lookups.size() - 1;
}

uint QV4::Compiler::JSUnitGenerator::registerGlobalGetterLookup(const QString &name)
{
    CompiledData::Lookup l;
    l.type_and_flags = CompiledData::Lookup::Type_GlobalGetter;
    l.nameIndex = registerString(name);
    lookups << l;
    return lookups.size() - 1;
}

int QV4::Compiler::JSUnitGenerator::registerRegExp(QQmlJS::V4IR::RegExp *regexp)
{
    CompiledData::RegExp re;
    re.stringIndex = registerString(*regexp->value);

    re.flags = 0;
    if (regexp->flags & QQmlJS::V4IR::RegExp::RegExp_Global)
        re.flags |= CompiledData::RegExp::RegExp_Global;
    if (regexp->flags & QQmlJS::V4IR::RegExp::RegExp_IgnoreCase)
        re.flags |= CompiledData::RegExp::RegExp_IgnoreCase;
    if (regexp->flags & QQmlJS::V4IR::RegExp::RegExp_Multiline)
        re.flags |= CompiledData::RegExp::RegExp_Multiline;

    regexps.append(re);
    return regexps.size() - 1;
}

void QV4::Compiler::JSUnitGenerator::registerLineNumberMapping(QQmlJS::V4IR::Function *function, const QVector<uint> &mappings)
{
    lineNumberMappingsPerFunction.insert(function, mappings);
}

int QV4::Compiler::JSUnitGenerator::registerJSClass(QQmlJS::V4IR::ExprList *args)
{
    // ### re-use existing class definitions.

    QList<CompiledData::JSClassMember> members;

    QQmlJS::V4IR::ExprList *it = args;
    while (it) {
        CompiledData::JSClassMember member;

        QQmlJS::V4IR::Name *name = it->expr->asName();
        it = it->next;

        const bool isData = it->expr->asConst()->value;
        it = it->next;

        member.nameOffset = registerString(*name->id);
        member.isAccessor = !isData;
        members << member;

        if (!isData)
            it = it->next;

        it = it->next;
    }

    jsClasses << members;
    jsClassDataSize += CompiledData::JSClass::calculateSize(members.count());
    return jsClasses.size() - 1;
}

QV4::CompiledData::Unit *QV4::Compiler::JSUnitGenerator::generateUnit()
{
    registerString(irModule->fileName);
    foreach (QQmlJS::V4IR::Function *f, irModule->functions) {
        registerString(*f->name);
        for (int i = 0; i < f->formals.size(); ++i)
            registerString(*f->formals.at(i));
        for (int i = 0; i < f->locals.size(); ++i)
            registerString(*f->locals.at(i));
    }

    int unitSize = QV4::CompiledData::Unit::calculateSize(strings.size(), irModule->functions.size(), regexps.size(), lookups.size(), jsClasses.count());

    uint functionDataSize = 0;
    for (int i = 0; i < irModule->functions.size(); ++i) {
        QQmlJS::V4IR::Function *f = irModule->functions.at(i);
        functionOffsets.insert(f, functionDataSize + unitSize + stringDataSize);

        int lineNumberMappingCount = 0;
        QHash<QQmlJS::V4IR::Function *, QVector<uint> >::ConstIterator lineNumberMapping = lineNumberMappingsPerFunction.find(f);
        if (lineNumberMapping != lineNumberMappingsPerFunction.constEnd())
            lineNumberMappingCount = lineNumberMapping->count() / 2;

        functionDataSize += QV4::CompiledData::Function::calculateSize(f->formals.size(), f->locals.size(), f->nestedFunctions.size(), lineNumberMappingCount);
    }

    char *data = (char *)malloc(unitSize + functionDataSize + stringDataSize + jsClassDataSize);
    QV4::CompiledData::Unit *unit = (QV4::CompiledData::Unit*)data;

    memcpy(unit->magic, QV4::CompiledData::magic_str, sizeof(unit->magic));
    unit->architecture = 0; // ###
    unit->flags = QV4::CompiledData::Unit::IsJavascript;
    unit->version = 1;
    unit->stringTableSize = strings.size();
    unit->offsetToStringTable = sizeof(QV4::CompiledData::Unit);
    unit->functionTableSize = irModule->functions.size();
    unit->offsetToFunctionTable = unit->offsetToStringTable + unit->stringTableSize * sizeof(uint);
    unit->lookupTableSize = lookups.count();
    unit->offsetToLookupTable = unit->offsetToFunctionTable + unit->functionTableSize * sizeof(uint);
    unit->regexpTableSize = regexps.size();
    unit->offsetToRegexpTable = unit->offsetToLookupTable + unit->lookupTableSize * CompiledData::Lookup::calculateSize();
    unit->jsClassTableSize = jsClasses.count();
    unit->offsetToJSClassTable = unit->offsetToRegexpTable + unit->regexpTableSize * CompiledData::RegExp::calculateSize();
    unit->sourceFileIndex = getStringId(irModule->fileName);

    // write strings and string table
    uint *stringTable = (uint *)(data + unit->offsetToStringTable);
    char *string = data + unitSize;
    for (int i = 0; i < strings.size(); ++i) {
        stringTable[i] = string - data;
        const QString &qstr = strings.at(i);

        QV4::CompiledData::String *s = (QV4::CompiledData::String*)(string);
        s->hash = QV4::String::createHashValue(qstr.constData(), qstr.length());
        s->flags = 0; // ###
        s->str.ref.atomic.store(-1);
        s->str.size = qstr.length();
        s->str.alloc = 0;
        s->str.capacityReserved = false;
        s->str.offset = sizeof(QArrayData);
        memcpy(s + 1, qstr.constData(), (qstr.length() + 1)*sizeof(ushort));

        string += QV4::CompiledData::String::calculateSize(qstr);
    }

    uint *functionTable = (uint *)(data + unit->offsetToFunctionTable);
    for (uint i = 0; i < irModule->functions.size(); ++i)
        functionTable[i] = functionOffsets.value(irModule->functions.at(i));

    char *f = data + unitSize + stringDataSize;
    for (uint i = 0; i < irModule->functions.size(); ++i) {
        QQmlJS::V4IR::Function *function = irModule->functions.at(i);
        if (function == irModule->rootFunction)
            unit->indexOfRootFunction = i;

        const int bytes = writeFunction(f, i, function);
        f += bytes;
    }

    CompiledData::Lookup *lookupsToWrite = (CompiledData::Lookup*)(data + unit->offsetToLookupTable);
    foreach (const CompiledData::Lookup &l, lookups)
        *lookupsToWrite++ = l;

    CompiledData::RegExp *regexpTable = (CompiledData::RegExp *)(data + unit->offsetToRegexpTable);
    memcpy(regexpTable, regexps.constData(), regexps.size() * sizeof(*regexpTable));

    // write js classes and js class lookup table
    uint *jsClassTable = (uint*)(data + unit->offsetToJSClassTable);
    char *jsClass = data + unitSize + stringDataSize + functionDataSize;
    for (int i = 0; i < jsClasses.count(); ++i) {
        jsClassTable[i] = jsClass - data;

        const QList<CompiledData::JSClassMember> members = jsClasses.at(i);

        CompiledData::JSClass *c = reinterpret_cast<CompiledData::JSClass*>(jsClass);
        c->nMembers = members.count();

        CompiledData::JSClassMember *memberToWrite = reinterpret_cast<CompiledData::JSClassMember*>(jsClass + sizeof(CompiledData::JSClass));
        foreach (const CompiledData::JSClassMember &member, members)
            *memberToWrite++ = member;

        jsClass += CompiledData::JSClass::calculateSize(members.count());
    }

    return unit;
}

int QV4::Compiler::JSUnitGenerator::writeFunction(char *f, int index, QQmlJS::V4IR::Function *irFunction)
{
    QV4::CompiledData::Function *function = (QV4::CompiledData::Function *)f;

    QHash<QQmlJS::V4IR::Function *, QVector<uint> >::ConstIterator lineNumberMapping = lineNumberMappingsPerFunction.find(irFunction);

    function->index = index;
    function->nameIndex = getStringId(*irFunction->name);
    function->flags = 0;
    if (irFunction->hasDirectEval)
        function->flags |= CompiledData::Function::HasDirectEval;
    if (irFunction->usesArgumentsObject)
        function->flags |= CompiledData::Function::UsesArgumentsObject;
    if (irFunction->isStrict)
        function->flags |= CompiledData::Function::IsStrict;
    if (irFunction->isNamedExpression)
        function->flags |= CompiledData::Function::IsNamedExpression;
    function->nFormals = irFunction->formals.size();
    function->formalsOffset = sizeof(QV4::CompiledData::Function);
    function->nLocals = irFunction->locals.size();
    function->localsOffset = function->formalsOffset + function->nFormals * sizeof(quint32);

    function->nLineNumberMappingEntries = 0;
    if (lineNumberMapping != lineNumberMappingsPerFunction.constEnd()) {
        function->nLineNumberMappingEntries = lineNumberMapping->count() / 2;
    }
    function->lineNumberMappingOffset = function->localsOffset + function->nLocals * sizeof(quint32);

    function->nInnerFunctions = irFunction->nestedFunctions.size();
    function->innerFunctionsOffset = function->lineNumberMappingOffset + function->nLineNumberMappingEntries * 2 * sizeof(quint32);

    // write formals
    quint32 *formals = (quint32 *)(f + function->formalsOffset);
    for (int i = 0; i < irFunction->formals.size(); ++i)
        formals[i] = getStringId(*irFunction->formals.at(i));

    // write locals
    quint32 *locals = (quint32 *)(f + function->localsOffset);
    for (int i = 0; i < irFunction->locals.size(); ++i)
        locals[i] = getStringId(*irFunction->locals.at(i));

    // write line number mappings
    if (function->nLineNumberMappingEntries) {
        quint32 *mappingsToWrite = (quint32*)(f + function->lineNumberMappingOffset);
        memcpy(mappingsToWrite, lineNumberMapping->constData(), 2 * function->nLineNumberMappingEntries * sizeof(quint32));
    }

    // write inner functions
    quint32 *innerFunctions = (quint32 *)(f + function->innerFunctionsOffset);
    for (int i = 0; i < irFunction->nestedFunctions.size(); ++i)
        innerFunctions[i] = functionOffsets.value(irFunction->nestedFunctions.at(i));

    return CompiledData::Function::calculateSize(function->nFormals, function->nLocals, function->nInnerFunctions, function->nLineNumberMappingEntries);
}



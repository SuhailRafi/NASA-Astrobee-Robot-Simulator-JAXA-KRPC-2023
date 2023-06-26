
"""
Common configuration for working with XPJSON and RAPID.
"""

import re

from xgds_planner2 import xpjson

XPJSON_PARAM_VALUE_TYPE_MAPPINGS = {
    'boolean': 'RAPID_BOOL',
    'double': 'RAPID_DOUBLE',
    'float': 'RAPID_FLOAT',
    'long': 'RAPID_INT',
    'long long': 'RAPID_LONG_LONG',
    'string': 'RAPID_STRING',
    'array[3].double': 'RAPID_VEC3d',
    'Point': 'RAPID_VEC3d',
    'array[9].float': 'RAPID_MAT33f',
    'quaternion': 'RAPID_MAT33f',
}

CAMEL_CASE_WORD_BOUNDARY_REGEX = re.compile(r'([a-z])([A-Z])')
NON_IDENT_CHARACTER_REGEX = re.compile(r'[^a-zA-Z0-9_]')


def fixName(s):
    """
    Hack. Some Astrobee commands have a parameter with id 'name_',
    which is a workaround for 'name' being a reserved word in XPJSON.
    For backward compatibility, we convert 'name_' to 'name' in some cases.
    """
    if s == 'name_':
        return 'name'
    else:
        return s


def allCaps(s):
    s = re.sub(CAMEL_CASE_WORD_BOUNDARY_REGEX,
               lambda m: m.group(1) + '_' + m.group(2), s)
    s = re.sub(NON_IDENT_CHARACTER_REGEX, '_', s)
    s = s.upper()
    return s


def loadDocument(path):
    """
    Return the result of xpjson.loadDocument(), but first setting some
    configuration we prefer for Astrobee.
    """
    xpjson.CHECK_UNKNOWN_FIELDS = False  # suppress some warnings
    xpjson.KEEP_PARAM_SPECS = True  # inhibit deletion of paramSpecs during load
    return xpjson.loadDocument(path)

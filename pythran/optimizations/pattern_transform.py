""" Optimization for Python costly pattern. """

from pythran.conversion import mangle
from pythran.analyses import Check, Placeholder
from pythran.passmanager import Transformation

from copy import deepcopy
import gast as ast
import sys

if sys.version_info.major == 3:
    range_name = 'range'
else:
    range_name = 'xrange'


class Pattern(object):

    def match(self, node):
        self.check = Check(node, dict())
        return self.check.visit(self.pattern)

    def replace(self):
        return PlaceholderReplace(self.check.placeholders).visit(self.sub())

    def imports(self):
        return deepcopy(getattr(self, 'extra_imports', []))


class LenSetPattern(Pattern):
    # __builtin__.len(__builtin__.set(X)) => __builtin__.pythran.len_set(X)
    pattern = ast.Call(func=ast.Attribute(value=ast.Name('__builtin__',
                                                         ast.Load(),
                                                         None, None),
                                          attr="len", ctx=ast.Load()),
                       args=[ast.Call(
                           func=ast.Attribute(
                               value=ast.Name('__builtin__', ast.Load(),
                                              None, None),
                               attr="set", ctx=ast.Load()),
                           args=[Placeholder(0)],
                           keywords=[])],
                       keywords=[])

    @staticmethod
    def sub():
        return ast.Call(
            func=ast.Attribute(
                value=ast.Attribute(value=ast.Name('__builtin__', ast.Load(),
                                                   None, None),
                                    attr="pythran", ctx=ast.Load()),
                attr="len_set", ctx=ast.Load()),
            args=[Placeholder(0)], keywords=[])


class TupleListPattern(Pattern):
    # __builtin__.tuple(__builtin__.list(X)) => __builtin__.tuple(X)

    pattern = ast.Call(func=ast.Attribute(value=ast.Name('__builtin__',
                                                         ast.Load(),
                                                         None, None),
                                          attr="tuple", ctx=ast.Load()),
                       args=[ast.Call(
                           func=ast.Attribute(
                               value=ast.Name('__builtin__',
                                              ast.Load(), None, None),
                               attr="list", ctx=ast.Load()),
                           args=[Placeholder(0)],
                           keywords=[])],
                       keywords=[])

    @staticmethod
    def sub():
        return ast.Call(
            func=ast.Attribute(value=ast.Name(id='__builtin__',
                                              ctx=ast.Load(),
                                              annotation=None,
                                              type_comment=None),
                               attr="tuple", ctx=ast.Load()),
            args=[Placeholder(0)], keywords=[])


class AbsSqrPattern(Pattern):
    # __builtin__.abs(X) ** 2 => __builtin__.pythran.abssqr(X)

    pattern = ast.Call(func=ast.Attribute(value=ast.Name(id=mangle('numpy'),
                                                         ctx=ast.Load(),
                                                         annotation=None,
                                                         type_comment=None),
                                          attr="square", ctx=ast.Load()),
                       args=[ast.Call(func=ast.Attribute(
                           value=ast.Name(id='__builtin__',
                                          ctx=ast.Load(),
                                          annotation=None,
                                          type_comment=None),
                           attr="abs",
                           ctx=ast.Load()),
                           args=[Placeholder(0)],
                           keywords=[])],
                       keywords=[])

    @staticmethod
    def sub():
        return ast.Call(
            func=ast.Attribute(
                value=ast.Attribute(value=ast.Name(id='__builtin__',
                                                   ctx=ast.Load(),
                                                   annotation=None,
                                                   type_comment=None),
                                    attr="pythran", ctx=ast.Load()),
                attr="abssqr", ctx=ast.Load()),
            args=[Placeholder(0)], keywords=[])


class AbsSqrPatternNumpy(AbsSqrPattern):
    # numpy.abs(X) ** 2 => __builtin__.pythran.abssqr(X)

    pattern = ast.Call(func=ast.Attribute(value=ast.Name(id=mangle('numpy'),
                                                         ctx=ast.Load(),
                                                         annotation=None,
                                                         type_comment=None),
                                          attr="square", ctx=ast.Load()),
                       args=[ast.Call(func=ast.Attribute(
                           value=ast.Name(id=mangle('numpy'),
                                          ctx=ast.Load(),
                                          annotation=None,
                                          type_comment=None),
                           attr="abs",
                           ctx=ast.Load()),
                           args=[Placeholder(0)],
                           keywords=[])],
                       keywords=[])


class SqrtPattern(Pattern):
    # X ** .5 => numpy.sqrt(X)

    pattern = ast.BinOp(Placeholder(0), ast.Pow(), ast.Constant(0.5, None))

    @staticmethod
    def sub():
        return ast.Call(
            func=ast.Attribute(value=ast.Name(id=mangle('numpy'),
                                              ctx=ast.Load(),
                                              annotation=None,
                                              type_comment=None),
                               attr="sqrt", ctx=ast.Load()),
            args=[Placeholder(0)], keywords=[])

    extra_imports = [ast.Import([ast.alias('numpy', mangle('numpy'))])]


class CbrtPattern(Pattern):
    # X ** .33333 => numpy.cbrt(X)
    pattern = ast.BinOp(Placeholder(0), ast.Pow(), ast.Constant(1./3., None))

    @staticmethod
    def sub():
        return ast.Call(
            func=ast.Attribute(value=ast.Name(id=mangle('numpy'),
                                              ctx=ast.Load(),
                                              annotation=None,
                                              type_comment=None),
                               attr="cbrt", ctx=ast.Load()),
            args=[Placeholder(0)], keywords=[])

    extra_imports = [ast.Import([ast.alias('numpy', mangle('numpy'))])]


class TuplePattern(Pattern):
    # __builtin__.tuple([X, ..., Z]) => (X, ..., Z)
    pattern = ast.Call(func=ast.Attribute(value=ast.Name(id='__builtin__',
                                                         ctx=ast.Load(),
                                                         annotation=None,
                                                         type_comment=None),
                                          attr="tuple", ctx=ast.Load()),
                       args=[ast.List(Placeholder(0), ast.Load())],
                       keywords=[])

    @staticmethod
    def sub():
        return ast.Tuple(Placeholder(0), ast.Load())


class ReversedRangePattern(Pattern):
    # __builtin__.reversed(__builtin__.xrange(X)) =>
    # __builtin__.xrange(X-1, -1, -1)
    # FIXME : We should do it even when begin/end/step are given
    pattern = ast.Call(func=ast.Attribute(value=ast.Name(id='__builtin__',
                                                         ctx=ast.Load(),
                                                         annotation=None,
                                                         type_comment=None),
                                          attr="reversed", ctx=ast.Load()),
                       args=[ast.Call(
                           func=ast.Attribute(
                               value=ast.Name(id='__builtin__',
                                              ctx=ast.Load(), annotation=None,
                                              type_comment=None),
                               attr=range_name, ctx=ast.Load()),
                           args=[Placeholder(0)],
                           keywords=[])],
                       keywords=[])

    @staticmethod
    def sub():
        return ast.Call(
            func=ast.Attribute(value=ast.Name(id='__builtin__',
                                              ctx=ast.Load(), annotation=None,
                                              type_comment=None),
                               attr=range_name, ctx=ast.Load()),
            args=[ast.BinOp(left=Placeholder(0), op=ast.Sub(),
                            right=ast.Constant(1, None)),
                  ast.Constant(-1, None),
                  ast.Constant(-1, None)],
            keywords=[])


class SqrPattern(Pattern):
    # X * X => X ** 2
    pattern = ast.BinOp(left=Placeholder(0),
                        op=ast.Mult(),
                        right=Placeholder(0))

    @staticmethod
    def sub():
        return ast.BinOp(left=Placeholder(0), op=ast.Pow(),
                         right=ast.Constant(2, None))


class StrJoinPattern(Pattern):
    # a + "..." + b => "...".join((a, b))
    pattern = ast.BinOp(left=ast.BinOp(left=Placeholder(0),
                                       op=ast.Add(),
                                       right=ast.Constant(Placeholder(1, str),
                                                          None)),
                        op=ast.Add(),
                        right=Placeholder(2))

    @staticmethod
    def sub():
        return ast.Call(func=ast.Attribute(
            ast.Attribute(
                ast.Name('__builtin__', ast.Load(), None, None),
                'str',
                ast.Load()),
            'join', ast.Load()),
            args=[ast.Constant(Placeholder(1), None),
                  ast.Tuple([Placeholder(0), Placeholder(2)], ast.Load())],
            keywords=[])


know_pattern = [x for x in globals().values() if hasattr(x, "pattern")]


class PlaceholderReplace(Transformation):

    """ Helper class to replace the placeholder once value is collected. """

    def __init__(self, placeholders):
        """ Store placeholders value collected. """
        self.placeholders = placeholders
        super(PlaceholderReplace, self).__init__()

    def visit(self, node):
        """ Replace the placeholder if it is one or continue. """
        if isinstance(node, Placeholder):
            return self.placeholders[node.id]
        else:
            return super(PlaceholderReplace, self).visit(node)


class PatternTransform(Transformation):

    """
    Replace all known pattern by pythran function call.

    Based on BaseMatcher to search correct pattern.
    """

    def __init__(self):
        """ Initialize the Basematcher to search for placeholders. """
        super(PatternTransform, self).__init__()

    def visit_Module(self, node):
        self.extra_imports = []
        self.generic_visit(node)
        node.body = self.extra_imports + node.body
        return node

    def visit(self, node):
        """ Try to replace if node match the given pattern or keep going. """
        for pattern in know_pattern:
            matcher = pattern()
            if matcher.match(node):
                self.extra_imports.extend(matcher.imports())
                node = matcher.replace()
                self.update = True

        return super(PatternTransform, self).visit(node)

# ParkinDSF

This is a Redis plugin that adds DSF (disjoint set forest) support to the server.

## Command Usage

Add 3 elements to the DSF...

```
> DSFADD mydsf red green blue
OK
```

All 3 elements are now members of their own singleton sets within the DSF.  To confirm this, we might issue...

```
> DSFCARD mydsf
(integer) 3
```

The cardinality of the DSF is indeed 3, but we can reduce it to 2 by merging two of the singleton sets into one set...

```
> DSFUNION mydsf green blue
OK
```

Did the cardinality decrease?

```
> DSFCARD mydsf
(integer) 2
```

Yup!  But there are 3 elements in the DSF...

```
> DSFSIZE mydsf
(integer) 3
```

At this point, `green` and `blue` are in one set, and `red` is in a set by itself.  Accordingly, ...

```
> DSFARECOMEMBERS mydsf green blue
(integer) 1
> DSFARECOMEMBERS mydsf blue red
(integer) 0
> DSFARECOMEMBERS mydsf green red
(integer) 0
```

The contents of the set containing a given element can be found as follows.

```
> DSFFINDSET mydsf red
1) red
> DSFFINDSET mydsf blue
1) blue
2) green
```

Membership in the DSF can also be tested, and elements can be removed, but these are not key features of the DSF data-structure.  In particular, removal does not have good time-complexity, but taking the union of sets, and testing whether two elements are members of the same set, each are very efficient.

## Command Syntax

```
DSFADD key element [element ...]
```

Add one or more elements to the DSF at key.  If no DSF exists at the given key, and the key is free, a DSF is created.

```
DSFREMOVE key element
```

Remove an element from the DSF at key.  If the DSF becomes empty after removal, the key is deleted.

```
DSFARECOMEMBERS key element1 element2
```

Test for membership in the same set within the DSF.  If the given elements are in the same set, a non-zero integer is returned; zero, otherwise.

```
DSFISMEMBER key element
```

Test for membership within the DSF.  If the given element is in the DSF, a non-zero integer is returned; zero, otherwise.

```
DSFUNION key element1 element2
```

Merge the two sets containing the given elements.  If both elements are already members of the same set, no operation is performed.

```
DSFCARD key
```

Return the number of sets within the DSF.

```
DSFSIZE key
```

Return the number of elements within the DSF.

```
DSFFINDSET key element
```

Return all elements in the set of the DSF containing the given element.

```
DSFDUMP key
```

Return the entire structure of the DSF as a list of lists.

## Note About Time-Complexity

Co-membership and union are the key features here, and their time-complexity is `O(ln N)`, where `N` is the number of elements within the DSF.  This may, however, seem disappointing.  These operations are said to have, on average, `O(1)` time-complexity.  This is true.  For Redis, however, we must tell it what elements we're interested in as strings.  Before Redis can apply DSF-algorithms, it must dereference those strings, and this is where the `O(ln N)` cost comes in.  For now, I'm not sure of a way around this.  In any case, the DSF data-structure should be fast enough for many applications.  Admittedly, I can't really think of any applications, but maybe you can.

All that said, the find-set and removal commands were added for completeness, but they are inherently and relatively innefficient operations on a DSF, and should probably be avoided unless your DSFs aren't too big.  Both are `O(N ln N)`, as are the operations of saving and loading a DSF to/from disk.

Beware: the dump command currently has worst-case time-complexity `O(N^2)`.  It's provided mainly for debugging purposes.
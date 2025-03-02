module Lambda where

import Data.List (nub, (\\))

data Lambda = Var String
            | App Lambda Lambda
            | Abs String Lambda
            | Macro String

instance Show Lambda where
    show (Var x) = x
    show (App e1 e2) = "(" ++ show e1 ++ " " ++ show e2 ++ ")"
    show (Abs x e) = "λ" ++ x ++ "." ++ show e
    show (Macro x) = x

instance Eq Lambda where
    e1 == e2 = eq e1 e2 ([],[],[])
      where
        eq (Var x) (Var y) (env,xb,yb) = elem (x,y) env || (not $ elem x xb || elem y yb)
        eq (App e1 e2) (App f1 f2) env = eq e1 f1 env && eq e2 f2 env
        eq (Abs x e) (Abs y f) (env,xb,yb) = eq e f ((x,y):env,x:xb,y:yb)
        eq (Macro x) (Macro y) _ = x == y
        eq _ _ _ = False

-- 1.1.
vars :: Lambda -> [String]
vars (Var var) = [var]
vars (App lb1 lb2) = vars lb1 ++ vars lb2
vars (Abs var lb) = var : filter (/= var) (vars lb)

-- 1.2.
freeVars :: Lambda -> [String]
freeVars (Var var) = [var]
freeVars (App lb1 lb2) = freeVars lb1 ++ filter (`notElem` freeVars lb1) (freeVars lb2)
freeVars (Abs var lb) = filter (/= var) (freeVars lb)
-- 1.3.
newVar :: [String] -> String
newVar str = aux alphabet (drop 1 $ aux_alphabet alphabet)
    where
        alphabet = "":map (:[]) ['a'..'z']
        aux_alphabet alpha = [a++b | a <- alphabet, b <- alpha]
        aux alpha (x:xs) = if x `elem` str
                           then aux alpha xs
                           else x
        aux alpha [] = aux (aux_alphabet alpha) (drop 1 $ aux_alphabet alpha)

-- 1.4.
isNormalForm :: Lambda -> Bool
isNormalForm (Var var) = True
isNormalForm (App lb1 lb2) = case lb1 of
        (Abs var lb) -> False
        _ -> isNormalForm lb1 && isNormalForm lb2
isNormalForm (Abs var lb) = isNormalForm lb

-- 1.5.
reduce :: String -> Lambda -> Lambda -> Lambda
reduce name (Var var) lb = if name == var
                           then lb
                           else Var var
reduce name (App lb1 lb2) lb3 = App (reduce name lb1 lb3) (reduce name lb2 lb3)
reduce name (Abs var lb1) lb2
    | var `elem` freeVars lb2 = reduce name (changeVar (Abs var lb1) var (newVar $ freeVars lb1)) lb2
    | var == name = Abs var lb1
    | otherwise = Abs var (reduce name lb1 lb2)
    where
        changeVar :: Lambda -> String -> String -> Lambda
        changeVar (Var var) old_name new_name = if old_name == var
                                                then Var new_name
                                                else Var var
        changeVar (App lb1 lb2) old_name new_name = App (changeVar lb1 old_name new_name) (changeVar lb2 old_name new_name)
        changeVar (Abs var lb) old_name new_name = if old_name == var
                                                   then Abs new_name (changeVar lb old_name new_name)
                                                   else Abs var (changeVar lb old_name new_name)

-- 1.6.
normalStep :: Lambda -> Lambda
normalStep (Var var) = Var var
normalStep (App lb1 lb2) = case lb1 of
        (Abs var lb) -> reduce var lb lb2
        _ -> if isNormalForm lb1
             then App lb1 (normalStep lb2)
             else App (normalStep lb1) lb2
normalStep (Abs var lb) = Abs var (normalStep lb)

-- 1.7.
applicativeStep :: Lambda -> Lambda
applicativeStep (Var var) = Var var
applicativeStep (App lb1 lb2) = case lb1 of
        (Abs var lb) -> if isNormalForm lb2
                        then reduce var lb lb2
                        else App lb1 (applicativeStep lb2)
        _            -> if isNormalForm lb1
                        then App lb1 (applicativeStep lb2)
                        else App (applicativeStep lb1) lb2
applicativeStep (Abs var lb) = Abs var (applicativeStep lb)

-- 1.8.
simplify :: (Lambda -> Lambda) -> Lambda -> [Lambda]
simplify step expr = nub $ aux_simplify [expr]
    where
        aux_simplify (x:xs) = if isNormalForm x
            then reverse (step x:x:xs)
            else aux_simplify (step x:x:xs)


normal :: Lambda -> [Lambda]
normal = simplify normalStep

applicative :: Lambda -> [Lambda]
applicative = simplify applicativeStep

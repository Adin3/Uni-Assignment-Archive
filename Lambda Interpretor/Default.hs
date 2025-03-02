module Default where

import Lambda
import Binding

-- Variables (for convenience)
vx = Var "x"
vy = Var "y"
vz = Var "z"
vf = Var "f"
vg = Var "g"
vh = Var "h"
vm = Var "m"
vn = Var "n"

-- Basic combinators
m = Abs "x" $ App vx vx
i = Abs "x" $ vx
k = Abs "x" $ Abs "y" $ vx
ki = Abs "x" $ Abs "y" $ vy
c = Abs "x" $ Abs "y" $ Abs "z" $ App (App vx vz) vy
y = Abs "f" $ App fix fix
  where fix = Abs "x" $ App vf (App vx vx)

-- 4.1. Boolean encodings
bTrue = Abs "x" $ Abs "y" vx
bFalse = Abs "x" $ Abs "y" vy
bAnd = Abs "x" $ Abs "y" $ App (App vx vy) vx
bOr = Abs "x" $ Abs "y" $ App (App vx vx) vy
bNot = Abs "x" $ App (App vx bFalse) bTrue
bXor = Abs "x" $ Abs "y" $ App (App vx (App bNot vy)) vy

-- 4.2. Pair encodings
pair = Abs "x" $ Abs "y" $ Abs "z" $ App (App vz vx) vy
first = Abs "m" $ App vm bTrue
second = Abs "m" $ App vm bFalse

-- 4.3. Natural number encodings
n0 = Abs "f" $ Abs "y" vy
n1 = Abs "f" $ Abs "y" $ App vf vy
n2 = Abs "f" $ Abs "y" $ App vf (App vf vy)
n3 = Abs "f" $ Abs "y" $ App vf (App vf (App vf vy))
n4 = Abs "f" $ Abs "y" $ App vf (App vf (App vf (App vf vy)))
nSucc =  Abs "n" $ Abs "f" $ Abs "y" $ App vf (App (App vn vf) vy)
nPred = Abs "n" $ Abs "f" $ Abs "x" $ App (App (App vn (Abs "g" $ Abs "h" $ App vh (App vg vf))) (Abs "z" vx)) (Abs "z" vz)
nAdd = Abs "m" $ Abs "n" $ App (App vn nSucc) vm
nSub = Abs "m" $ Abs "n" $ App (App vn nPred) vm
nMult = Abs "m" $ Abs "n" $ Abs "f" $ App vm (App vn vf)

-- Default Context
defaultContext :: Context
defaultContext = 
    [ ("M", m)
    , ("I", i)
    , ("K", k)
    , ("KI", ki)
    , ("C", c)
    , ("Y", y)
    , ("TRUE", bTrue)
    , ("FALSE", bFalse)
    , ("AND", bAnd)
    , ("OR", bOr)
    , ("NOT", bNot)
    , ("XOR", bXor)
    , ("PAIR", pair)
    , ("FST", first)
    , ("SND", second)
    , ("N0", n0)
    , ("N1", n1)
    , ("N2", n2)
    , ("SUCC", nSucc)
    , ("PRED", nPred)
    ,("ADD", nAdd)
    , ("SUB", nSub)
    , ("MULT", nMult)
    ]

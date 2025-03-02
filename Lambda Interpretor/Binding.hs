module Binding where

import Lambda

type Context = [(String, Lambda)]

data Line = Eval Lambda
          | Binding String Lambda deriving (Eq)

instance Show Line where
    show (Eval l) = show l
    show (Binding s l) = s ++ " = " ++ show l

changeMacro :: Context -> Lambda -> Either String Lambda
changeMacro ctx lb = case lb of
    (Macro var) -> case lookup var ctx of
        Nothing -> Left "ERROR"
        Just x ->  Right x
    (Var var) -> Right $ Var var
    (App lb1 lb2) -> do
        m1 <- changeMacro ctx lb1
        m2 <- changeMacro ctx lb2
        return $ App m1 m2
    (Abs var lb1) -> do
        m <- changeMacro ctx lb1
        return $ Abs var m

-- 3.1.
simplifyCtx :: Context -> (Lambda -> Lambda) -> Lambda -> Either String [Lambda]
simplifyCtx ctx step lb = case changeMacro ctx lb of
                        Left x -> Left x
                        Right x -> Right $ simplify step x

normalCtx :: Context -> Lambda -> Either String [Lambda]
normalCtx ctx = simplifyCtx ctx normalStep

applicativeCtx :: Context -> Lambda -> Either String [Lambda]
applicativeCtx ctx = simplifyCtx ctx applicativeStep

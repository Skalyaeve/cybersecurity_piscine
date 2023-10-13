CREATE DATABASE test;
GO
USE test;
GO
CREATE TABLE users (
    id INT PRIMARY KEY IDENTITY(1,1),
    username NVARCHAR(255),
    password NVARCHAR(255)
);
GO
CREATE TABLE superusers (
    id INT PRIMARY KEY IDENTITY(1,1),
    username NVARCHAR(255),
    password NVARCHAR(255),
    superkey NVARCHAR(255)
);
GO
INSERT INTO users (username, password)
VALUES ('user', 'password4');
GO
INSERT INTO users (username, password)
VALUES ('admin', '<!v3rY_sTr0nG_aDmN1N_pAsSwOrD![ 4 ]>');
GO
INSERT INTO superusers (username, password)
VALUES ('superadmin', '<!!!v3rY_sTr0nG_aDmN1N_pAsSwOrD!!![ 4 ]>', '123');
GO



# Гайд по сдаче домашек

При регистрации в Manytask вам был создан аккаунт в Gitlab, а также создан приватный репозиторий, куда вам нужно будет посылать решения. Сдача домашек происходит путём коммита решений через Git.

## Добавление SSH-ключей
Узнать свой публичный ключ можно посмотрев его в директории с SSH-ключами: `~/.ssh`. Обычно он называется `id_rsa.pub` и посмотреть его можно вот так:
```
cat ~/.ssh/id_rsa.pub
```
Если публичного ключа нет, то сгенерировать его можно вот так:
```
ssh-keygen -t rsa -b 4096
```
После этого заходите [сюда](https://gitlab.carzil.ru/-/profile/keys), нажимаете **Add new key**, копируете ключ в поле **Key**, пишете любое название в **Title** и нажимаете **Add key**.

## Клонирование репозитория
После добавление SSH-ключа можно попробовать склонировать репозиторий:
```
git clone git@gitlab.carzil.ru:mipt-os-basic-2024-autumn/<ваш ник>.git
```
Клон будет создан в текущей директории и будет называться `<ваш ник>`.

## Добавление апстрима
При появлении новых задач или тестов, необходимо будет эти обновления забрать к себе в локальный репоизиторий. Для этого **единожды** после клонирования нужно добавить апстрим в локальный репозиторий:
```
git remote add upstream git@gitlab.carzil.ru:mipt-os-basic/public-2024.git
```
Забрать изменения из публичного репозитория:
```
git pull upstream main
```

## Сдача задач
После изменения файлов задач их нужно закоммитить:
```
git add -u
git commit -m "<любое сообщение коммита>"
```
И отослать (пушнуть) в GitLab:
```
git push
```
После этого в вашем репозитории появятся коммиты, которые будут протестированы. Результат тестирования можно посмотреть во вкладке **Build → Pipelines** или рядом с коммитом.

# GIL Ransomware Solution

랜섬웨어로 부터 중요 파일을 보호하는 FsMiniFilter Driver의 핵심 부분을 정리한 프로젝트 (진행중)

## Feature
- DMZ Folder: 접근에 제한된 폴더
- Super Process: DMZ Folder에 접근할 수 있는 프로세스

## Build
.

## Installation
- Driver Install

    .inf를 통해 설치

- Driver load & unload

    ```
    fltmc load GilRan
    
    fltmc unload GilRan
    ```
#pragma once

#include <unordered_map>
#include <memory>
#include <iostream>
#include <type_traits>
#include <SFML/Graphics.hpp>

namespace game
{
    using TFontsMap = std::unordered_map<int, std::shared_ptr<sf::Font>>;
    using TFonstMapIterator = std::unordered_map<int, std::shared_ptr<sf::Font>>::iterator;
    
    using TTexturesMap = std::unordered_map<int, std::shared_ptr<sf::Texture>>;
    using TTexturesMapIterator = std::unordered_map<int, std::shared_ptr<sf::Texture>>::iterator;

    using TSpritesMap =  std::unordered_map<int, std::shared_ptr<sf::Sprite>>;
    using TSpritesMapIterator =  std::unordered_map<int, std::shared_ptr<sf::Sprite>>::iterator;
    
    using TTextsMap = std::unordered_map<int, std::shared_ptr<sf::Text>>;
    using TTextsMapIterator = std::unordered_map<int, std::shared_ptr<sf::Text>>::iterator;

    template<class T>
    struct MapTypeTraits
    {
    };

    template<>
    struct MapTypeTraits<sf::Font>
    {
        typedef TFontsMap map_type;
        typedef TFonstMapIterator iterator_type;
    };

    template<>
    struct MapTypeTraits<sf::Texture>
    {
        typedef TTexturesMap map_type;
        typedef TTexturesMapIterator iterator_type;
    };

    template<>
    struct MapTypeTraits<sf::Sprite>
    {
        typedef TSpritesMap map_type;
        typedef TSpritesMapIterator iterator_type;
    };

    template<>
    struct MapTypeTraits<sf::Text>
    {
        typedef TTextsMap map_type;
        typedef TTextsMapIterator iterator_type;
    };

    class CResourceManager
    {
    public:
        static CResourceManager& getInstance()
        {
            static CResourceManager instance;
            return instance;
        }

        template<
                class T,
                typename MapTraits = MapTypeTraits<T>, 
                class ...Args
            >
        std::shared_ptr<T> addResource(const int resID, Args&&... args)
        {
            bool result = false;
            typename MapTraits::map_type& map = getMap<T>();

            std::shared_ptr<T> retVal;
            
            if(map.find(resID) == map.end())
            {
                std::pair<typename MapTraits::iterator_type, bool> ret; 
                ret = map.emplace(resID, std::make_shared<T>(std::forward<Args>(args)...));
                retVal = ret.first->second;
            }
            else
            {
                retVal = map.at(resID);
            }
            
            return retVal;
        }

        template<
                class T,
                typename MapTraits = MapTypeTraits<T>
            >
        std::shared_ptr<T> getResource(const int resID)
        {
            typename MapTraits::map_type& map = getMap<T>();
            if(map.find(resID) != map.end())
            {
                return map.at(resID);
            }
            else
            {
                return std::shared_ptr<T>();
            }   
        }

        template<
                class T,
                typename MapTraits = MapTypeTraits<T>
            >
        const T* getResourceFast(const int resID)
        {
            T* retVal = nullptr;
            typename MapTraits::map_type& map = getMap<T>();
            if(map.find(resID) != map.end())
            {
                retVal = map.at(resID).get();
            }
            return retVal;
        }


    private:
        CResourceManager() = default;
        ~CResourceManager() = default;
        CResourceManager(const CResourceManager&) = delete;
        CResourceManager& operator=(const CResourceManager& other) = delete;

        template<class T, typename std::enable_if_t<std::is_same<T, sf::Font>::value>* = nullptr>
        TFontsMap& getMap()  
        {
             return mFontsMap;
        }

        template<class T, typename std::enable_if_t<std::is_same<T, sf::Texture>::value>* = nullptr>
        TTexturesMap& getMap()  
        {
             return mTexturesMap;
        }

        template<class T, typename std::enable_if_t<std::is_same<T, sf::Sprite>::value>* = nullptr>
        TSpritesMap& getMap()  
        {
             return mSpritesMap;
        }

        template<class T, typename std::enable_if_t<std::is_same<T, sf::Text>::value>* = nullptr>
        TTextsMap& getMap()  
        {
             return mTextsMap;
        }

    private:
        TFontsMap mFontsMap;
        TTexturesMap mTexturesMap;
        TSpritesMap mSpritesMap;
        TTextsMap mTextsMap;
    };        
}